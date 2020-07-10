/*  Copyright (C) 2019-2020 Aria Salvatrice
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

// Self-modifying sequencer. Internally, the slots are called "nodes", "step" refers to the movement.
// Templates are used to create multiple versions: 4, 8, and 16 steps.

// TODO: Make Reset work
// TODO: Implement Save/Load buttons
// TODO: Implement Total Nodes knob
// TODO: Make notes flash upon trigger
// TODO: Make LCD work..... cleaner than last modules
// TODO: On reset
// TODO: On randomize
// TODO: JSON I/O
// TODO: Portable sequences

#include "plugin.hpp"
#include "lcd.hpp"
#include "quantizer.hpp"

namespace Solomon {

const float READWINDOWDURATION = 0.001f; // Seconds
const int OUTPUTDIVIDER = 32;

enum StepTypes {
    STEP_QUEUE,
    STEP_TELEPORT,
    STEP_WALK,
    STEP_BACK,
    STEP_FORWARD
};

enum LcdModes {
    INIT_MODE,
    SCALE_MODE,
    MINMAX_MODE,
    TOTAL_NODES_MODE,
    SLIDE_MODE
};

template <size_t NODES>
struct Solomon : Module {
    enum ParamIds {
        KEY_PARAM,
        SCALE_PARAM,
        MIN_PARAM,
        MAX_PARAM,
        SLIDE_PARAM,
        TOTAL_NODES_PARAM,
        QUEUE_CLEAR_MODE_PARAM,
        SAVE_PARAM,
        LOAD_PARAM,
        ENUMS(NODE_SUB_1_SD_PARAM, NODES),
        ENUMS(NODE_ADD_1_SD_PARAM, NODES),
        ENUMS(NODE_QUEUE_PARAM, NODES),
        NUM_PARAMS
    };
    enum InputIds {
        EXT_SCALE_INPUT,
        STEP_QUEUE_INPUT,
        STEP_TELEPORT_INPUT,
        STEP_WALK_INPUT,
        STEP_BACK_INPUT,
        STEP_FORWARD_INPUT,
        RESET_INPUT,
        ENUMS(NODE_SUB_1_SD_INPUT, NODES),
        ENUMS(NODE_SUB_2_SD_INPUT, NODES),
        ENUMS(NODE_SUB_3_SD_INPUT, NODES),
        ENUMS(NODE_SUB_1_OCT_INPUT, NODES),
        ENUMS(NODE_ADD_1_SD_INPUT, NODES),
        ENUMS(NODE_ADD_2_SD_INPUT, NODES),
        ENUMS(NODE_ADD_3_SD_INPUT, NODES),
        ENUMS(NODE_ADD_1_OCT_INPUT, NODES),
        ENUMS(NODE_QUEUE_INPUT, NODES),
        NUM_INPUTS
    };
    enum OutputIds {
        GLOBAL_TRIG_OUTPUT,
        GLOBAL_CV_OUTPUT,
        ENUMS(NODE_GATE_OUTPUT, NODES),
        ENUMS(NODE_RANDOM_OUTPUT, NODES),
        ENUMS(NODE_LATCH_OUTPUT, NODES),
        ENUMS(NODE_DELAY_OUTPUT, NODES),
        ENUMS(NODE_CV_OUTPUT, NODES),
        NUM_OUTPUTS
    };
    enum LightIds {
        ENUMS(NODE_LIGHT, NODES),
        NUM_LIGHTS
    };

    // Global
    bool randomGate = false;
    int stepType = -1;
    size_t currentNode = 0;
    size_t selectedQueueNode = 0;
    float readWindow = -1.f; // -1 when closed
    std::array<bool, 12> scale;
    dsp::SchmittTrigger stepQueueTrigger;
    dsp::SchmittTrigger stepTeleportTrigger;
    dsp::SchmittTrigger stepWalkTrigger;
    dsp::SchmittTrigger stepBackTrigger;
    dsp::SchmittTrigger stepForwardTrigger;
    dsp::SchmittTrigger saveButtonTrigger;
    dsp::SchmittTrigger loadButtonTrigger;
    dsp::PulseGenerator globalTrig;
    dsp::ClockDivider outputDivider;
    Lcd::LcdStatus lcdStatus;

    // Per node
    float cv[NODES];
    float savedCv[NODES];
    std::array<bool, NODES> queue;
    std::array<bool, NODES> windowQueue;
    std::array<bool, NODES> delay;
    std::array<bool, NODES> sub1Sd;
    std::array<bool, NODES> sub2Sd;
    std::array<bool, NODES> sub3Sd;
    std::array<bool, NODES> sub1Oct;
    std::array<bool, NODES> add1Sd;
    std::array<bool, NODES> add2Sd;
    std::array<bool, NODES> add3Sd;
    std::array<bool, NODES> add1Oct;
    std::array<bool, NODES> latch;
    dsp::SchmittTrigger sub1SdTrigger[NODES];
    dsp::SchmittTrigger add1SdTrigger[NODES];
    dsp::SchmittTrigger queueTrigger[NODES];

    Solomon() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        configParam(MIN_PARAM, 0.f, 10.f, 3.f, "Minimum Note");
        configParam(MAX_PARAM, 0.f, 10.f, 5.f, "Maximum Note");
        configParam(SLIDE_PARAM, 0.f, 10.f, 0.f, "Slide");

        // C Minor is the default
        configParam(KEY_PARAM, 0.f, 11.f, 0.f, "Key");
        configParam(SCALE_PARAM, 0.f, (float) Quantizer::NUM_SCALES - 1, 2.f, "Scale");
        scale = Quantizer::validNotesInScaleKey(Quantizer::NATURAL_MINOR, 0);
        // Default note is 0V - C4 - part of the default scale
        for(size_t i = 0; i < NODES; i++) {
            cv[i] = 0.f;
            savedCv[i] = 0.f;
        }

        clearQueue();
        clearWindowQueue();
        clearDelay();
        clearTransposes();
        clearLatches();

        outputDivider.setDivision(OUTPUTDIVIDER);

        lcdStatus.lcdLayout = Lcd::TEXT1_AND_TEXT2_LAYOUT;
        lcdStatus.lcdMode = INIT_MODE;
        lcdStatus.lcdText1 = "LEARNING...";
        lcdStatus.lcdText2 = "SUMMONING..";
    }

    void updateScale() {
        if (inputs[EXT_SCALE_INPUT].isConnected() ) {
            for (size_t i = 0; i < 12; i++) scale[i] = (inputs[EXT_SCALE_INPUT].getVoltage(i) > 0.f) ? true : false;
        } else {
            scale = Quantizer::validNotesInScaleKey( (int) params[SCALE_PARAM].getValue(), (int) params[KEY_PARAM].getValue());
        }
    }

    // How many nodes are enqueued
    size_t queueCount() {
        size_t count = 0;
        for(size_t i = 0; i < NODES; i++) {
            if (queue[i] == true) count++;
        }
        return count;
    }

    // It's safe for users to swap Min and Max. Clamped to avoid C10 breaking the display.
    float getMinCv() {
        if(params[MIN_PARAM].getValue() <= params[MAX_PARAM].getValue()) {
            return clamp(params[MIN_PARAM].getValue() - 4.f, -4.f, 5.85f);
        } else {
            return clamp(params[MAX_PARAM].getValue() - 4.f, -4.f, 5.85f);
        }
    }

    // It's safe for users to swap Min and Max. Clamped to avoid C10 breaking the display.
    float getMaxCv() {
        if(params[MIN_PARAM].getValue() <= params[MAX_PARAM].getValue()) {
            return clamp(params[MAX_PARAM].getValue() - 4.f, -4.f, 5.85f);
        } else {
            return clamp(params[MIN_PARAM].getValue() - 4.f, -4.f, 5.85f);
        }
    }

    // Subtracts scale degrees. Wraps around on overflow.
    void subSd(size_t node, size_t sd) {
        if (cv[node] > getMaxCv()) {
            cv[node] = getMaxCv();
        }
        for (size_t i = 0; i < sd; i++) {
            cv[node] = Quantizer::quantize(cv[node], scale, - 1);
            if(cv[node] < getMinCv()) {
                cv[node] = Quantizer::quantize(getMaxCv(), scale);
            }
        }
    }

    // Adds scale degrees. Wraps around on overflow.
    void addSd(size_t node, size_t sd) {
        if (cv[node] < getMinCv()) {
            cv[node] = getMinCv();
        }
        for (size_t i = 0; i < sd; i++) {
            cv[node] = Quantizer::quantize(cv[node], scale, 1);
            if(cv[node] > getMaxCv()) {
                cv[node] = Quantizer::quantize(getMinCv(), scale);
            }
        }
    }

    // FIXME: Quantize - scale can have changed.
    // Does nothing if there's no valid note to jump to
    void subOct(size_t node) {       
        if (cv[node] - 1.f >= getMinCv() - Quantizer::FUDGEOFFSET) {
            // We can remove an octave and stay in bounds
            cv[node] -= 1.f;
        } else {
            // Separate octave from note
            float nodeOctave = floorf(cv[node]);
            float nodeVoltageOnFirstOctave = cv[node] - nodeOctave;
            float maxOctave = floorf(getMaxCv());
            float candidate = maxOctave + nodeVoltageOnFirstOctave; 
            if (candidate <= getMaxCv() + Quantizer::FUDGEOFFSET && candidate >= getMinCv() - Quantizer::FUDGEOFFSET) {
                // We can wrap around on the max octave
                cv[node] = candidate;
            } else {
                candidate -= 1.f;
                if (candidate <= getMaxCv() + Quantizer::FUDGEOFFSET && candidate >= getMinCv() - Quantizer::FUDGEOFFSET) {
                    // We can wrap around one octave lower than the max octave
                    cv[node] = candidate;
                }
            }
        }
    }

    // Does nothing if there's no valid note to jump to
    // Same code as above with + and - and min and max flipped flipways
    void addOct(size_t node) {
        if (cv[node] + 1.f <= getMaxCv() + Quantizer::FUDGEOFFSET) {
            cv[node] += 1.f;
        } else {
            float nodeOctave = floorf(cv[node]);
            float nodeVoltageOnFirstOctave = cv[node] - nodeOctave;
            float minOctave = floorf(getMinCv());
            float candidate = minOctave + nodeVoltageOnFirstOctave; 
            if (candidate >= getMinCv() - Quantizer::FUDGEOFFSET && candidate <= getMaxCv() + Quantizer::FUDGEOFFSET) {
                cv[node] = candidate;
            } else {
                candidate += 1.f;
                if (candidate >= getMinCv() - Quantizer::FUDGEOFFSET && candidate <= getMaxCv() + Quantizer::FUDGEOFFSET) {
                    cv[node] = candidate;
                }
            }
        }
    }

    // Each node has 2 manual - and + buttons that are processed whether in a window or not.
    void processSdButtons() {
        for (size_t i = 0; i < NODES; i++) {
            if(sub1SdTrigger[i].process(params[NODE_SUB_1_SD_PARAM + i].getValue())) {
                subSd(i, 1);
            }
            if(add1SdTrigger[i].process(params[NODE_ADD_1_SD_PARAM + i].getValue())) {
                addSd(i, 1);
            }
        }
    }

    // Each node has a manual Q button that is processed whether in a window or not.
    // Unlike the CV, it toggles.
    void processQueueButtons() {
        for (size_t i = 0; i < NODES; i++) {
            if(queueTrigger[i].process(params[NODE_QUEUE_PARAM + i].getValue())) {
                queue[i] = ! queue[i];
            }
        }
    }

    void processLoadButton() {
        if(saveButtonTrigger.process(params[LOAD_PARAM].getValue())){
            for (size_t i = 0; i < NODES; i++) cv[i] = savedCv[i];
        }
    }

    void processSaveButton(){
        if(saveButtonTrigger.process(params[SAVE_PARAM].getValue())){
            for (size_t i = 0; i < NODES; i++) savedCv[i] = cv[i];
        }
    }

    // If it's a queue input, something must be already enqueued.
    int getStepInput() {
        if (stepQueueTrigger.process(inputs[STEP_QUEUE_INPUT].getVoltageSum()) && queueCount() > 0) return STEP_QUEUE;
        if (stepTeleportTrigger.process(inputs[STEP_TELEPORT_INPUT].getVoltageSum()))               return STEP_TELEPORT;
        if (stepWalkTrigger.process(inputs[STEP_WALK_INPUT].getVoltageSum()))                       return STEP_WALK;
        if (stepBackTrigger.process(inputs[STEP_BACK_INPUT].getVoltageSum()))                       return STEP_BACK;
        if (stepForwardTrigger.process(inputs[STEP_FORWARD_INPUT].getVoltageSum()))                 return STEP_FORWARD;
        return -1;
    }

    void clearQueue() {
        for(size_t i = 0; i < NODES; i++) queue[i] = false;
    }

    void clearWindowQueue() {
        for(size_t i = 0; i < NODES; i++) windowQueue[i] = false;
    }

    void clearDelay() {
        for(size_t i = 0; i < NODES; i++) delay[i] = false;
    }

    void clearLatches() {
        for(size_t i = 0; i < NODES; i++) latch[i] = false;
    }

    // During Read Windows, see if we received queue messages.
    void readWindowQueue() {
        for(size_t i = 0; i < NODES; i++) {
            if (inputs[NODE_QUEUE_INPUT + i].getVoltageSum() > 0.f) {
                windowQueue[i] = true;
            }
        }
    }

    // Store what we selected out of the queue, remove it from the queue, and reset the queue if switch set to reset.
    void applyQueue() {

        // Only select from the queue if we know we have something in it, or we crash.
        if (stepType == STEP_QUEUE) {
            std::vector<size_t> validSteps;
            for (size_t i = 0; i < NODES; i++) {
                if (queue[i]) validSteps.push_back(i);
            }
            std::random_shuffle(validSteps.begin(), validSteps.end());
            selectedQueueNode = validSteps[0];
            queue[selectedQueueNode] = false;
        }
        
        // Clear queue if requested
        if (params[QUEUE_CLEAR_MODE_PARAM].getValue() == 1.f) clearQueue();

        // Add window queue triggers no matter the configuration        
        for (size_t i = 0; i < NODES; i++) {
            if (!queue[i]) queue[i] = windowQueue[i];
        }
        clearWindowQueue();
    }

    void clearTransposes() {
        for(size_t i = 0; i < NODES; i++) {
            sub1Sd[i]  = false;
            sub2Sd[i]  = false;
            sub3Sd[i]  = false;
            sub1Oct[i] = false;
            add1Sd[i]  = false;
            add2Sd[i]  = false;
            add3Sd[i]  = false;
            add1Oct[i] = false;
        }
    }

    // Doesn't need to be proper triggers.
    void readTransposes() {
        for(size_t i = 0; i < NODES; i++) {
            if (inputs[NODE_SUB_1_SD_INPUT  + i].getVoltageSum() > 0.f)  sub1Sd[i] = true;
            if (inputs[NODE_SUB_2_SD_INPUT  + i].getVoltageSum() > 0.f)  sub2Sd[i] = true;
            if (inputs[NODE_SUB_3_SD_INPUT  + i].getVoltageSum() > 0.f)  sub3Sd[i] = true;
            if (inputs[NODE_SUB_1_OCT_INPUT + i].getVoltageSum() > 0.f) sub1Oct[i] = true;
            if (inputs[NODE_ADD_1_SD_INPUT  + i].getVoltageSum() > 0.f)  add1Sd[i] = true;
            if (inputs[NODE_ADD_2_SD_INPUT  + i].getVoltageSum() > 0.f)  add2Sd[i] = true;
            if (inputs[NODE_ADD_3_SD_INPUT  + i].getVoltageSum() > 0.f)  add3Sd[i] = true;
            if (inputs[NODE_ADD_1_OCT_INPUT + i].getVoltageSum() > 0.f) add1Oct[i] = true;
        }
    }

    void applyTransposes() {
        for(size_t i = 0; i < NODES; i++) {
            if (sub1Sd[i] ) subSd(i, 1);
            if (sub2Sd[i] ) subSd(i, 2);
            if (sub3Sd[i] ) subSd(i, 8);
            if (sub1Oct[i]) subOct(i);
            if (add1Sd[i] ) addSd(i, 1);
            if (add2Sd[i] ) addSd(i, 2);
            if (add3Sd[i] ) addSd(i, 9);
            if (add1Oct[i]) addOct(i);
        }
        clearTransposes();
    }

    void applyDelay() {
        for(size_t i = 0; i < NODES; i++) delay[i] = false;
        delay[currentNode] = true;
    }

    void applyStep() {
        // If we reach this, we already have a selectedQueueNode
        if (stepType == STEP_QUEUE) {
            currentNode = selectedQueueNode;
        }

        // Teleport never brings back to the current step
        if (stepType == STEP_TELEPORT) {
            std::vector<size_t> validNodes;
            for (size_t i = 0; i < NODES; i++) {
                if (i != currentNode) validNodes.push_back(i);
            }
            std::random_shuffle(validNodes.begin(), validNodes.end());
            currentNode = validNodes[0];
        }

        // Random walk can warp around
        if (stepType == STEP_WALK) {
            if (random::uniform() >= 0.5f) {
                // Walk forward
                if (currentNode == NODES - 1) {
                    currentNode = 0;
                } else {
                    currentNode++;
                }
            } else {
                // Walk back
                if (currentNode == 0) {
                    currentNode = NODES - 1;
                } else {
                    currentNode--;
                }
            }
        }

        // Step back can warp around
        if (stepType == STEP_BACK) {
            if (currentNode == 0) {
                currentNode = NODES - 1;
            } else {
                currentNode--;
            }
        }

        // Step forward can warp around
        if (stepType == STEP_FORWARD) {
            if (currentNode == NODES - 1) {
                currentNode = 0;
            } else {
                currentNode++;
            }
        }
    }

    void updateLatch() {
        latch[currentNode] = !latch[currentNode];
    }

    void processReadWindow() {
        readWindowQueue();
        readTransposes();
    }

    // A read window just elapsed, we move to the next step and send the outputs
    void processStep() {
        applyTransposes();
        applyQueue();
        applyDelay();
        applyStep();
        updateLatch();
        clearTransposes();
        randomGate = ( random::uniform() >= 0.5f ) ? true : false;
        globalTrig.trigger(1e-3f);
        stepType = -1;
    }

    // We refresh lotsa stuff, but we don't need to do it at audio rates
    void sendOutputs(const ProcessArgs& args) {
        outputs[GLOBAL_TRIG_OUTPUT].setVoltage( globalTrig.process(args.sampleTime) ? 10.f : 0.f);
        outputs[GLOBAL_CV_OUTPUT].setVoltage(cv[currentNode]); // TODO: Slide

        for(size_t i = 0; i < NODES; i++) {
            outputs[NODE_DELAY_OUTPUT + i].setVoltage( (delay[i]) ? 10.f : 0.f);
            outputs[NODE_CV_OUTPUT    + i].setVoltage( cv[i] );

            if (i == currentNode) {
                outputs[NODE_GATE_OUTPUT   + i].setVoltage(10.f);
                outputs[NODE_RANDOM_OUTPUT + i].setVoltage( (randomGate) ? 10.f : 0.f);
                outputs[NODE_LATCH_OUTPUT  + i].setVoltage( (latch[i]) ? 10.f : 0.f);
            } else {
                outputs[NODE_GATE_OUTPUT   + i].setVoltage(0.f );
                outputs[NODE_RANDOM_OUTPUT + i].setVoltage(0.f);
                outputs[NODE_LATCH_OUTPUT  + i].setVoltage(0.f);
            }
        }
    }

    void process(const ProcessArgs& args) override {

        if (readWindow < 0.f) {
            // We are not in a Read Window
            stepType = getStepInput();
            if (stepType >= 0) readWindow = 0.f;

        }
        if (readWindow >= 0.f && readWindow < READWINDOWDURATION) {
            // We are in a Read Window
            processReadWindow();
            readWindow += args.sampleTime;
        }
        if (readWindow >= READWINDOWDURATION) {
            // A read window closed
            processStep();
            readWindow = -1.f;
        }

        // No need to process this many outputs at audio rates, or to refresh those
        // inputs this often.
        if (outputDivider.process()) {
            sendOutputs(args);
            updateScale();
            processSdButtons();
            processQueueButtons();
            processLoadButton();
            processSaveButton();
        }

    }

};




///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////




// Total nodes knobs
template <typename TModule>
struct TotalNodesKnob : AriaKnob820 {
    void onDragMove(const event::DragMove& e) override {
        dynamic_cast<TModule*>(paramQuantity->module)->lcdStatus.lcdLastInteraction = 0.f;
        dynamic_cast<TModule*>(paramQuantity->module)->lcdStatus.lcdDirty = true;
        dynamic_cast<TModule*>(paramQuantity->module)->lcdStatus.lcdMode = TOTAL_NODES_MODE;
        AriaKnob820::onDragMove(e);
    }
};

// Scale/key knobs
template <typename TModule>
struct ScaleKnob : AriaKnob820 {
    ScaleKnob() {
        snap = true;
        AriaKnob820();
    }
    void onDragMove(const event::DragMove& e) override {
        dynamic_cast<TModule*>(paramQuantity->module)->lcdStatus.lcdLastInteraction = 0.f;
        dynamic_cast<TModule*>(paramQuantity->module)->lcdStatus.lcdDirty = true;
        dynamic_cast<TModule*>(paramQuantity->module)->lcdStatus.lcdMode = SCALE_MODE;
        AriaKnob820::onDragMove(e);
    }
};

// Min/Max knobs
template <typename TModule>
struct MinMaxKnob : AriaKnob820 {
    void onDragMove(const event::DragMove& e) override {
        dynamic_cast<TModule*>(paramQuantity->module)->lcdStatus.lcdLastInteraction = 0.f;
        dynamic_cast<TModule*>(paramQuantity->module)->lcdStatus.lcdDirty = true;
        dynamic_cast<TModule*>(paramQuantity->module)->lcdStatus.lcdMode = MINMAX_MODE;
        AriaKnob820::onDragMove(e);
    }
};

// Slide knobs
template <typename TModule>
struct SlideKnob : AriaKnob820 {
    void onDragMove(const event::DragMove& e) override {
        dynamic_cast<TModule*>(paramQuantity->module)->lcdStatus.lcdLastInteraction = 0.f;
        dynamic_cast<TModule*>(paramQuantity->module)->lcdStatus.lcdDirty = true;
        dynamic_cast<TModule*>(paramQuantity->module)->lcdStatus.lcdMode = SLIDE_MODE;
        AriaKnob820::onDragMove(e);
    }
};

// Per-node segment display
template <typename TModule>
struct SegmentDisplay : TransparentWidget {
	TModule* module;
    size_t node;
	std::shared_ptr<Font> font;
    std::string text = "";

	SegmentDisplay() {
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/dseg/DSEG14ClassicMini-Italic.ttf"));
	}

	void draw(const DrawArgs& args) override {
		nvgFontSize(args.vg, 20);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, 2.0);

        Vec textPos = mm2px(Vec(0.f, 10.f));
		nvgFillColor(args.vg, nvgRGB(0x0b, 0x57, 0x63));
		nvgText(args.vg, textPos.x, textPos.y, "~~~", NULL);
		nvgFillColor(args.vg, nvgRGB(0xc1, 0xf0, 0xf2));
        if(module) {
            text = Quantizer::noteOctaveSegmentName(module->cv[node]);
            nvgText(args.vg, textPos.x, textPos.y, text.c_str(), NULL);
        }
	}
};

template <typename TModule>
struct SegmentDisplayFramebuffer : FramebufferWidget {
    TModule* module;
    size_t node;
    float lastStatus = -20.f;

    void step() override{
        if (module) { 
            if (module->cv[node] != lastStatus) {
                dirty = true;
            }
            FramebufferWidget::step();
        }
    }
};


// The QUEUE message on the segment display
template <typename TModule>
struct QueueWidget : Widget {
	TModule* module;
    size_t node;
	FramebufferWidget* framebuffer;
	SvgWidget* svgWidget;
    bool lastStatus;

    QueueWidget() {
        framebuffer = new widget::FramebufferWidget;
        addChild(framebuffer);
        svgWidget = new widget::SvgWidget;
        svgWidget->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/solomon-queue-lit.svg")));
        framebuffer->box.size = svgWidget->box.size;
        box.size = svgWidget->box.size;
        framebuffer->addChild(svgWidget);
        lastStatus = true;
    }

    void step() override {
        if(module) {
            if (module->queue[node] != lastStatus) {
                framebuffer->visible = (module->queue[node] == true) ? true : false;
            }
            lastStatus = module->queue[node];
        }
        Widget::step();
    }
};

// The DELAY message on the segment display
template <typename TModule>
struct DelayWidget : Widget {
	TModule* module;
    size_t node;
	FramebufferWidget* framebuffer;
	SvgWidget* svgWidget;
    bool lastStatus;

    DelayWidget() {
        framebuffer = new widget::FramebufferWidget;
        addChild(framebuffer);
        svgWidget = new widget::SvgWidget;
        svgWidget->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/solomon-delay-lit.svg")));
        framebuffer->box.size = svgWidget->box.size;
        box.size = svgWidget->box.size;
        framebuffer->addChild(svgWidget);
        lastStatus = true;
    }

    void step() override {
        if(module) {
            if (module->queue[node] != lastStatus) {
                framebuffer->visible = (module->delay[node] == true) ? true : false;
            }
            lastStatus = module->delay[node];
        }
        Widget::step();
    }
};

// The PLAY arrow on the segment display
template <typename TModule>
struct PlayWidget : Widget {
	TModule* module;
    size_t node;
	FramebufferWidget* framebuffer;
	SvgWidget* svgWidget;
    size_t lastStatus; 

    PlayWidget() {
        framebuffer = new widget::FramebufferWidget;
        addChild(framebuffer);
        svgWidget = new widget::SvgWidget;
        svgWidget->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/solomon-play-lit.svg")));
        framebuffer->box.size = svgWidget->box.size;
        box.size = svgWidget->box.size;
        framebuffer->addChild(svgWidget);
        lastStatus = true;
    }

    void step() override {
        if(module) {
            if (module->currentNode != lastStatus) {
                framebuffer->visible = (module->currentNode == node) ? true : false;
            }
            lastStatus = module->currentNode;
        }
        Widget::step();
    }
};


// 8 is the main version, from which the others are copied
struct SolomonWidget8 : ModuleWidget {

    SolomonWidget8(Solomon<8>* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/faceplates/Solomon8.svg")));
        
        // Screws
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Signature
        addChild(createWidget<AriaSignature>(mm2px(Vec(37.5f, 114.5f))));

        // Queue clear mode
        addParam(createParam<AriaRockerSwitchVertical800>(mm2px(Vec(28.4f, 17.1f)), module, Solomon<8>::QUEUE_CLEAR_MODE_PARAM));

        // Global step inputs. Ordered counterclockwise.
        addInput(createInput<AriaJackIn>(mm2px(Vec(20.f, 17.f)), module, Solomon<8>::STEP_QUEUE_INPUT));
        addInput(createInput<AriaJackIn>(mm2px(Vec( 5.f, 32.f)), module, Solomon<8>::STEP_TELEPORT_INPUT));
        addInput(createInput<AriaJackIn>(mm2px(Vec(35.f, 32.f)), module, Solomon<8>::STEP_FORWARD_INPUT));
        addInput(createInput<AriaJackIn>(mm2px(Vec(10.f, 47.f)), module, Solomon<8>::STEP_WALK_INPUT));
        addInput(createInput<AriaJackIn>(mm2px(Vec(30.f, 47.f)), module, Solomon<8>::STEP_BACK_INPUT));

        // Total Steps
        addParam(createParam<MinMaxKnob<Solomon<8>>>(mm2px(Vec(20.f, 32.f)), module, Solomon<8>::TOTAL_NODES_PARAM));

        // LCD
        Lcd::LcdWidget<Solomon<8>> *lcd = new Lcd::LcdWidget<Solomon<8>>(module);
        lcd->box.pos = mm2px(Vec(7.7f, 68.8f));
        addChild(lcd);

        addParam(createParam<ScaleKnob<Solomon<8>>>(mm2px(Vec(15.f, 81.f)), module, Solomon<8>::KEY_PARAM));
        addParam(createParam<ScaleKnob<Solomon<8>>>(mm2px(Vec(27.f, 81.f)), module, Solomon<8>::SCALE_PARAM));
        addInput(createInput<AriaJackIn>(mm2px(Vec(39.f, 81.f)), module, Solomon<8>::EXT_SCALE_INPUT));

        addParam(createParam<MinMaxKnob<Solomon<8>>>(mm2px(Vec(15.f, 94.f)), module, Solomon<8>::MIN_PARAM));
        addParam(createParam<MinMaxKnob<Solomon<8>>>(mm2px(Vec(27.f, 94.f)), module, Solomon<8>::MAX_PARAM));
        addParam(createParam<SlideKnob<Solomon<8>>>(mm2px(Vec(39.f, 94.f)), module, Solomon<8>::SLIDE_PARAM));

        // Reset
        addInput(createInput<AriaJackIn>(mm2px(Vec(3.f, 107.f)), module, Solomon<8>::RESET_INPUT));

        // Global output
        addOutput(createOutput<AriaJackOut>(mm2px(Vec(15.f, 107.f)), module, Solomon<8>::GLOBAL_TRIG_OUTPUT));
        addOutput(createOutput<AriaJackOut>(mm2px(Vec(27.f, 107.f)), module, Solomon<8>::GLOBAL_CV_OUTPUT));

        // Load and Save
        addParam(createParam<AriaPushButton820Momentary>(mm2px(Vec(3.f, 81.f)), module, Solomon<8>::SAVE_PARAM));
        addParam(createParam<AriaPushButton820Momentary>(mm2px(Vec(3.f, 94.f)), module, Solomon<8>::LOAD_PARAM));

        // Nodes
        float xOffset = 53.f;
        float yOffset = 17.f;
        for(size_t i = 0; i < 8; i++) {
            // Inputs
            addInput(createInput<AriaJackIn>(mm2px(Vec(xOffset +  5.f, yOffset +  0.f)), module, Solomon<8>::NODE_QUEUE_INPUT     + i));
            addInput(createInput<AriaJackIn>(mm2px(Vec(xOffset +  0.f, yOffset + 10.f)), module, Solomon<8>::NODE_SUB_1_OCT_INPUT + i));
            addInput(createInput<AriaJackIn>(mm2px(Vec(xOffset +  0.f, yOffset + 20.f)), module, Solomon<8>::NODE_SUB_3_SD_INPUT  + i));
            addInput(createInput<AriaJackIn>(mm2px(Vec(xOffset +  0.f, yOffset + 30.f)), module, Solomon<8>::NODE_SUB_2_SD_INPUT  + i));
            addInput(createInput<AriaJackIn>(mm2px(Vec(xOffset +  0.f, yOffset + 40.f)), module, Solomon<8>::NODE_SUB_1_SD_INPUT  + i));
            addInput(createInput<AriaJackIn>(mm2px(Vec(xOffset + 10.f, yOffset + 10.f)), module, Solomon<8>::NODE_ADD_1_OCT_INPUT + i));
            addInput(createInput<AriaJackIn>(mm2px(Vec(xOffset + 10.f, yOffset + 20.f)), module, Solomon<8>::NODE_ADD_3_SD_INPUT  + i));
            addInput(createInput<AriaJackIn>(mm2px(Vec(xOffset + 10.f, yOffset + 30.f)), module, Solomon<8>::NODE_ADD_2_SD_INPUT  + i));
            addInput(createInput<AriaJackIn>(mm2px(Vec(xOffset + 10.f, yOffset + 40.f)), module, Solomon<8>::NODE_ADD_1_SD_INPUT  + i));

            // Segment Display
            SegmentDisplay<Solomon<8>>* display = new SegmentDisplay<Solomon<8>>();
            SegmentDisplayFramebuffer<Solomon<8>>* framebuffer = new SegmentDisplayFramebuffer<Solomon<8>>();
            display->module = module;
            display->node = i;
            framebuffer->module = module;
            framebuffer->node = i;
            display->box.size = mm2px(Vec(20.f, 10.f));
            framebuffer->box.pos = mm2px(Vec(xOffset + 0.f, yOffset + 48.f));
            framebuffer->addChild(display);
            addChild(framebuffer);
            QueueWidget<Solomon<8>>* queueWidget = new QueueWidget<Solomon<8>>;
            queueWidget->box.pos = mm2px(Vec(xOffset + 0.25f, yOffset + 59.0f));
            queueWidget->module = module;
            queueWidget->node = i;
            addChild(queueWidget);
            DelayWidget<Solomon<8>>* delayWidget = new DelayWidget<Solomon<8>>;
            delayWidget->box.pos = mm2px(Vec(xOffset + 9.85f, yOffset + 59.0f));
            delayWidget->module = module;
            delayWidget->node = i;
            addChild(delayWidget);
            PlayWidget<Solomon<8>>* playWidget = new PlayWidget<Solomon<8>>;
            playWidget->box.pos = mm2px(Vec(xOffset - 3.31f, yOffset + 51.25f));
            playWidget->module = module;
            playWidget->node = i;
            addChild(playWidget);

            // Buttons
            addParam(createParam<AriaPushButton820Momentary>(mm2px(Vec(xOffset +  0.f, yOffset + 64.f)), module, Solomon<8>::NODE_SUB_1_SD_PARAM + i));
            addParam(createParam<AriaPushButton820Momentary>(mm2px(Vec(xOffset + 10.f, yOffset + 64.f)), module, Solomon<8>::NODE_ADD_1_SD_PARAM + i));
            addParam(createParam<AriaPushButton820Momentary>(mm2px(Vec(xOffset +  5.f, yOffset + 71.f)), module, Solomon<8>::NODE_QUEUE_PARAM + i));

            // Outputs
            addOutput(createOutput<AriaJackOut>(mm2px(Vec(xOffset +  0.f, yOffset +  80.f)), module, Solomon<8>::NODE_GATE_OUTPUT + i));
            addOutput(createOutput<AriaJackOut>(mm2px(Vec(xOffset + 10.f, yOffset +  80.f)), module, Solomon<8>::NODE_RANDOM_OUTPUT  + i));
            addOutput(createOutput<AriaJackOut>(mm2px(Vec(xOffset +  5.f, yOffset +  88.f)), module, Solomon<8>::NODE_CV_OUTPUT + i));
            addOutput(createOutput<AriaJackOut>(mm2px(Vec(xOffset +  0.f, yOffset +  96.f)), module, Solomon<8>::NODE_LATCH_OUTPUT   + i));
            addOutput(createOutput<AriaJackOut>(mm2px(Vec(xOffset + 10.f, yOffset +  96.f)), module, Solomon<8>::NODE_DELAY_OUTPUT    + i));

            xOffset += 25.f;
        }
    }
};





///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////




struct SolomonWidget4 : ModuleWidget {

    SolomonWidget4(Solomon<4>* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/faceplates/Solomon4.svg")));
        
        // Screws
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Signature
        addChild(createWidget<AriaSignature>(mm2px(Vec(37.5f, 114.5f))));

        // Queue clear mode
        addParam(createParam<AriaRockerSwitchVertical800>(mm2px(Vec(28.4f, 17.1f)), module, Solomon<4>::QUEUE_CLEAR_MODE_PARAM));

        // Global step inputs. Ordered counterclockwise.
        addInput(createInput<AriaJackIn>(mm2px(Vec(20.f, 17.f)), module, Solomon<4>::STEP_QUEUE_INPUT));
        addInput(createInput<AriaJackIn>(mm2px(Vec( 5.f, 32.f)), module, Solomon<4>::STEP_TELEPORT_INPUT));
        addInput(createInput<AriaJackIn>(mm2px(Vec(35.f, 32.f)), module, Solomon<4>::STEP_FORWARD_INPUT));
        addInput(createInput<AriaJackIn>(mm2px(Vec(10.f, 47.f)), module, Solomon<4>::STEP_WALK_INPUT));
        addInput(createInput<AriaJackIn>(mm2px(Vec(30.f, 47.f)), module, Solomon<4>::STEP_BACK_INPUT));

        // Total Steps
        addParam(createParam<MinMaxKnob<Solomon<4>>>(mm2px(Vec(20.f, 32.f)), module, Solomon<4>::TOTAL_NODES_PARAM));

        // LCD
        Lcd::LcdWidget<Solomon<4>> *lcd = new Lcd::LcdWidget<Solomon<4>>(module);
        lcd->box.pos = mm2px(Vec(7.7f, 68.8f));
        addChild(lcd);

        addParam(createParam<ScaleKnob<Solomon<4>>>(mm2px(Vec(15.f, 81.f)), module, Solomon<4>::KEY_PARAM));
        addParam(createParam<ScaleKnob<Solomon<4>>>(mm2px(Vec(27.f, 81.f)), module, Solomon<4>::SCALE_PARAM));
        addInput(createInput<AriaJackIn>(mm2px(Vec(39.f, 81.f)), module, Solomon<4>::EXT_SCALE_INPUT));

        addParam(createParam<MinMaxKnob<Solomon<4>>>(mm2px(Vec(15.f, 94.f)), module, Solomon<4>::MIN_PARAM));
        addParam(createParam<MinMaxKnob<Solomon<4>>>(mm2px(Vec(27.f, 94.f)), module, Solomon<4>::MAX_PARAM));
        addParam(createParam<SlideKnob<Solomon<4>>>(mm2px(Vec(39.f, 94.f)), module, Solomon<4>::SLIDE_PARAM));

        // Reset
        addInput(createInput<AriaJackIn>(mm2px(Vec(3.f, 107.f)), module, Solomon<4>::RESET_INPUT));

        // Global output
        addOutput(createOutput<AriaJackOut>(mm2px(Vec(15.f, 107.f)), module, Solomon<4>::GLOBAL_TRIG_OUTPUT));
        addOutput(createOutput<AriaJackOut>(mm2px(Vec(27.f, 107.f)), module, Solomon<4>::GLOBAL_CV_OUTPUT));

        // Load and Save
        addParam(createParam<AriaPushButton820Momentary>(mm2px(Vec(3.f, 81.f)), module, Solomon<4>::SAVE_PARAM));
        addParam(createParam<AriaPushButton820Momentary>(mm2px(Vec(3.f, 94.f)), module, Solomon<4>::LOAD_PARAM));

        // Nodes
        float xOffset = 53.f;
        float yOffset = 17.f;
        for(size_t i = 0; i < 4; i++) {
            // Inputs
            addInput(createInput<AriaJackIn>(mm2px(Vec(xOffset +  5.f, yOffset +  0.f)), module, Solomon<4>::NODE_QUEUE_INPUT     + i));
            addInput(createInput<AriaJackIn>(mm2px(Vec(xOffset +  0.f, yOffset + 10.f)), module, Solomon<4>::NODE_SUB_1_OCT_INPUT + i));
            addInput(createInput<AriaJackIn>(mm2px(Vec(xOffset +  0.f, yOffset + 20.f)), module, Solomon<4>::NODE_SUB_3_SD_INPUT  + i));
            addInput(createInput<AriaJackIn>(mm2px(Vec(xOffset +  0.f, yOffset + 30.f)), module, Solomon<4>::NODE_SUB_2_SD_INPUT  + i));
            addInput(createInput<AriaJackIn>(mm2px(Vec(xOffset +  0.f, yOffset + 40.f)), module, Solomon<4>::NODE_SUB_1_SD_INPUT  + i));
            addInput(createInput<AriaJackIn>(mm2px(Vec(xOffset + 10.f, yOffset + 10.f)), module, Solomon<4>::NODE_ADD_1_OCT_INPUT + i));
            addInput(createInput<AriaJackIn>(mm2px(Vec(xOffset + 10.f, yOffset + 20.f)), module, Solomon<4>::NODE_ADD_3_SD_INPUT  + i));
            addInput(createInput<AriaJackIn>(mm2px(Vec(xOffset + 10.f, yOffset + 30.f)), module, Solomon<4>::NODE_ADD_2_SD_INPUT  + i));
            addInput(createInput<AriaJackIn>(mm2px(Vec(xOffset + 10.f, yOffset + 40.f)), module, Solomon<4>::NODE_ADD_1_SD_INPUT  + i));

            // Segment Display
            SegmentDisplay<Solomon<4>>* display = new SegmentDisplay<Solomon<4>>();
            SegmentDisplayFramebuffer<Solomon<4>>* framebuffer = new SegmentDisplayFramebuffer<Solomon<4>>();
            display->module = module;
            display->node = i;
            framebuffer->module = module;
            framebuffer->node = i;
            display->box.size = mm2px(Vec(20.f, 10.f));
            framebuffer->box.pos = mm2px(Vec(xOffset + 0.f, yOffset + 48.f));
            framebuffer->addChild(display);
            addChild(framebuffer);
            QueueWidget<Solomon<4>>* queueWidget = new QueueWidget<Solomon<4>>;
            queueWidget->box.pos = mm2px(Vec(xOffset + 0.25f, yOffset + 59.0f));
            queueWidget->module = module;
            queueWidget->node = i;
            addChild(queueWidget);
            DelayWidget<Solomon<4>>* delayWidget = new DelayWidget<Solomon<4>>;
            delayWidget->box.pos = mm2px(Vec(xOffset + 9.85f, yOffset + 59.0f));
            delayWidget->module = module;
            delayWidget->node = i;
            addChild(delayWidget);
            PlayWidget<Solomon<4>>* playWidget = new PlayWidget<Solomon<4>>;
            playWidget->box.pos = mm2px(Vec(xOffset - 3.31f, yOffset + 51.25f));
            playWidget->module = module;
            playWidget->node = i;
            addChild(playWidget);

            // Buttons
            addParam(createParam<AriaPushButton820Momentary>(mm2px(Vec(xOffset +  0.f, yOffset + 64.f)), module, Solomon<4>::NODE_SUB_1_SD_PARAM + i));
            addParam(createParam<AriaPushButton820Momentary>(mm2px(Vec(xOffset + 10.f, yOffset + 64.f)), module, Solomon<4>::NODE_ADD_1_SD_PARAM + i));
            addParam(createParam<AriaPushButton820Momentary>(mm2px(Vec(xOffset +  5.f, yOffset + 71.f)), module, Solomon<4>::NODE_QUEUE_PARAM + i));

            // Outputs
            addOutput(createOutput<AriaJackOut>(mm2px(Vec(xOffset +  0.f, yOffset +  80.f)), module, Solomon<4>::NODE_GATE_OUTPUT + i));
            addOutput(createOutput<AriaJackOut>(mm2px(Vec(xOffset + 10.f, yOffset +  80.f)), module, Solomon<4>::NODE_RANDOM_OUTPUT  + i));
            addOutput(createOutput<AriaJackOut>(mm2px(Vec(xOffset +  5.f, yOffset +  88.f)), module, Solomon<4>::NODE_CV_OUTPUT + i));
            addOutput(createOutput<AriaJackOut>(mm2px(Vec(xOffset +  0.f, yOffset +  96.f)), module, Solomon<4>::NODE_LATCH_OUTPUT   + i));
            addOutput(createOutput<AriaJackOut>(mm2px(Vec(xOffset + 10.f, yOffset +  96.f)), module, Solomon<4>::NODE_DELAY_OUTPUT    + i));

            xOffset += 25.f;
        }
    }
};




///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////



struct SolomonWidget16 : ModuleWidget {

    SolomonWidget16(Solomon<16>* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/faceplates/Solomon16.svg")));
        
        // Screws
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Signature
        addChild(createWidget<AriaSignature>(mm2px(Vec(37.5f, 114.5f))));

        // Queue clear mode
        addParam(createParam<AriaRockerSwitchVertical800>(mm2px(Vec(28.4f, 17.1f)), module, Solomon<16>::QUEUE_CLEAR_MODE_PARAM));

        // Global step inputs. Ordered counterclockwise.
        addInput(createInput<AriaJackIn>(mm2px(Vec(20.f, 17.f)), module, Solomon<16>::STEP_QUEUE_INPUT));
        addInput(createInput<AriaJackIn>(mm2px(Vec( 5.f, 32.f)), module, Solomon<16>::STEP_TELEPORT_INPUT));
        addInput(createInput<AriaJackIn>(mm2px(Vec(35.f, 32.f)), module, Solomon<16>::STEP_FORWARD_INPUT));
        addInput(createInput<AriaJackIn>(mm2px(Vec(10.f, 47.f)), module, Solomon<16>::STEP_WALK_INPUT));
        addInput(createInput<AriaJackIn>(mm2px(Vec(30.f, 47.f)), module, Solomon<16>::STEP_BACK_INPUT));

        // Total Steps
        addParam(createParam<MinMaxKnob<Solomon<16>>>(mm2px(Vec(20.f, 32.f)), module, Solomon<16>::TOTAL_NODES_PARAM));

        // LCD
        Lcd::LcdWidget<Solomon<16>> *lcd = new Lcd::LcdWidget<Solomon<16>>(module);
        lcd->box.pos = mm2px(Vec(7.7f, 68.8f));
        addChild(lcd);

        addParam(createParam<ScaleKnob<Solomon<16>>>(mm2px(Vec(15.f, 81.f)), module, Solomon<16>::KEY_PARAM));
        addParam(createParam<ScaleKnob<Solomon<16>>>(mm2px(Vec(27.f, 81.f)), module, Solomon<16>::SCALE_PARAM));
        addInput(createInput<AriaJackIn>(mm2px(Vec(39.f, 81.f)), module, Solomon<16>::EXT_SCALE_INPUT));

        addParam(createParam<MinMaxKnob<Solomon<16>>>(mm2px(Vec(15.f, 94.f)), module, Solomon<16>::MIN_PARAM));
        addParam(createParam<MinMaxKnob<Solomon<16>>>(mm2px(Vec(27.f, 94.f)), module, Solomon<16>::MAX_PARAM));
        addParam(createParam<SlideKnob<Solomon<16>>>(mm2px(Vec(39.f, 94.f)), module, Solomon<16>::SLIDE_PARAM));

        // Reset
        addInput(createInput<AriaJackIn>(mm2px(Vec(3.f, 107.f)), module, Solomon<16>::RESET_INPUT));

        // Global output
        addOutput(createOutput<AriaJackOut>(mm2px(Vec(15.f, 107.f)), module, Solomon<16>::GLOBAL_TRIG_OUTPUT));
        addOutput(createOutput<AriaJackOut>(mm2px(Vec(27.f, 107.f)), module, Solomon<16>::GLOBAL_CV_OUTPUT));

        // Load and Save
        addParam(createParam<AriaPushButton820Momentary>(mm2px(Vec(3.f, 81.f)), module, Solomon<16>::SAVE_PARAM));
        addParam(createParam<AriaPushButton820Momentary>(mm2px(Vec(3.f, 94.f)), module, Solomon<16>::LOAD_PARAM));

        // Nodes
        float xOffset = 53.f;
        float yOffset = 17.f;
        for(size_t i = 0; i < 16; i++) {
            // Inputs
            addInput(createInput<AriaJackIn>(mm2px(Vec(xOffset +  5.f, yOffset +  0.f)), module, Solomon<16>::NODE_QUEUE_INPUT     + i));
            addInput(createInput<AriaJackIn>(mm2px(Vec(xOffset +  0.f, yOffset + 10.f)), module, Solomon<16>::NODE_SUB_1_OCT_INPUT + i));
            addInput(createInput<AriaJackIn>(mm2px(Vec(xOffset +  0.f, yOffset + 20.f)), module, Solomon<16>::NODE_SUB_3_SD_INPUT  + i));
            addInput(createInput<AriaJackIn>(mm2px(Vec(xOffset +  0.f, yOffset + 30.f)), module, Solomon<16>::NODE_SUB_2_SD_INPUT  + i));
            addInput(createInput<AriaJackIn>(mm2px(Vec(xOffset +  0.f, yOffset + 40.f)), module, Solomon<16>::NODE_SUB_1_SD_INPUT  + i));
            addInput(createInput<AriaJackIn>(mm2px(Vec(xOffset + 10.f, yOffset + 10.f)), module, Solomon<16>::NODE_ADD_1_OCT_INPUT + i));
            addInput(createInput<AriaJackIn>(mm2px(Vec(xOffset + 10.f, yOffset + 20.f)), module, Solomon<16>::NODE_ADD_3_SD_INPUT  + i));
            addInput(createInput<AriaJackIn>(mm2px(Vec(xOffset + 10.f, yOffset + 30.f)), module, Solomon<16>::NODE_ADD_2_SD_INPUT  + i));
            addInput(createInput<AriaJackIn>(mm2px(Vec(xOffset + 10.f, yOffset + 40.f)), module, Solomon<16>::NODE_ADD_1_SD_INPUT  + i));

            // Segment Display
            SegmentDisplay<Solomon<16>>* display = new SegmentDisplay<Solomon<16>>();
            SegmentDisplayFramebuffer<Solomon<16>>* framebuffer = new SegmentDisplayFramebuffer<Solomon<16>>();
            display->module = module;
            display->node = i;
            framebuffer->module = module;
            framebuffer->node = i;
            display->box.size = mm2px(Vec(20.f, 10.f));
            framebuffer->box.pos = mm2px(Vec(xOffset + 0.f, yOffset + 48.f));
            framebuffer->addChild(display);
            addChild(framebuffer);
            QueueWidget<Solomon<16>>* queueWidget = new QueueWidget<Solomon<16>>;
            queueWidget->box.pos = mm2px(Vec(xOffset + 0.25f, yOffset + 59.0f));
            queueWidget->module = module;
            queueWidget->node = i;
            addChild(queueWidget);
            DelayWidget<Solomon<16>>* delayWidget = new DelayWidget<Solomon<16>>;
            delayWidget->box.pos = mm2px(Vec(xOffset + 9.85f, yOffset + 59.0f));
            delayWidget->module = module;
            delayWidget->node = i;
            addChild(delayWidget);
            PlayWidget<Solomon<16>>* playWidget = new PlayWidget<Solomon<16>>;
            playWidget->box.pos = mm2px(Vec(xOffset - 3.31f, yOffset + 51.25f));
            playWidget->module = module;
            playWidget->node = i;
            addChild(playWidget);

            // Buttons
            addParam(createParam<AriaPushButton820Momentary>(mm2px(Vec(xOffset +  0.f, yOffset + 64.f)), module, Solomon<16>::NODE_SUB_1_SD_PARAM + i));
            addParam(createParam<AriaPushButton820Momentary>(mm2px(Vec(xOffset + 10.f, yOffset + 64.f)), module, Solomon<16>::NODE_ADD_1_SD_PARAM + i));
            addParam(createParam<AriaPushButton820Momentary>(mm2px(Vec(xOffset +  5.f, yOffset + 71.f)), module, Solomon<16>::NODE_QUEUE_PARAM + i));

            // Outputs
            addOutput(createOutput<AriaJackOut>(mm2px(Vec(xOffset +  0.f, yOffset +  80.f)), module, Solomon<16>::NODE_GATE_OUTPUT + i));
            addOutput(createOutput<AriaJackOut>(mm2px(Vec(xOffset + 10.f, yOffset +  80.f)), module, Solomon<16>::NODE_RANDOM_OUTPUT  + i));
            addOutput(createOutput<AriaJackOut>(mm2px(Vec(xOffset +  5.f, yOffset +  88.f)), module, Solomon<16>::NODE_CV_OUTPUT + i));
            addOutput(createOutput<AriaJackOut>(mm2px(Vec(xOffset +  0.f, yOffset +  96.f)), module, Solomon<16>::NODE_LATCH_OUTPUT   + i));
            addOutput(createOutput<AriaJackOut>(mm2px(Vec(xOffset + 10.f, yOffset +  96.f)), module, Solomon<16>::NODE_DELAY_OUTPUT    + i));

            xOffset += 25.f;
        }
    }
};


} // Namespace Solomon

Model* modelSolomon4 = createModel<Solomon::Solomon<4>, Solomon::SolomonWidget4>("Solomon4");
Model* modelSolomon8 = createModel<Solomon::Solomon<8>, Solomon::SolomonWidget8>("Solomon8");
Model* modelSolomon16 = createModel<Solomon::Solomon<16>, Solomon::SolomonWidget16>("Solomon16");
