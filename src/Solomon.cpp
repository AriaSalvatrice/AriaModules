/*  Copyright (C) 2019-2020 Aria Salvatrice
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

// Self-modifying sequencer. Internally, the slots are called "nodes", "step" refers to the movement.
// Templates are used to create multiple versions: 4, 8, and 16 steps.

#include "plugin.hpp"
#include "lcd.hpp"
#include "quantizer.hpp"
#include "prng.hpp"
#include "portablesequence.hpp"

namespace Solomon {

const float READWINDOWDURATION = 0.001f; // Seconds
const float WINDOWTIMEOUTDURATION = 0.002f; // How fast windows can open
const int OUTPUTDIVIDER = 32;

enum StepTypes {
    STEP_QUEUE,
    STEP_TELEPORT,
    STEP_WALK,
    STEP_BACK,
    STEP_FORWARD
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
        REPEAT_MODE_PARAM,
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
    bool copyPortableSequence = false;
    bool pastePortableSequence = false;
    bool resetStepConfig = true;
    bool resetLoadConfig = true;
    bool resetQuantizeConfig = false;
    bool randomizePitchesRequested = false;
    bool quantizePitchesRequested = false;
    int stepType = -1;
    size_t currentNode = 0;
    size_t selectedQueueNode = 0;
    float readWindow = -1.f; // -1 when closed
    float resetDelay = -1.f; // 0 when reset started
    float windowTimeout = 0.f; // Wait between accepting triggers
    float slideDuration = 0.f;
    float slideCounter = 0.f;
    float lastOutput = 0.f;
    std::array<bool, 12> scale;
    dsp::SchmittTrigger stepQueueTrigger;
    dsp::SchmittTrigger stepTeleportTrigger;
    dsp::SchmittTrigger stepWalkTrigger;
    dsp::SchmittTrigger stepBackTrigger;
    dsp::SchmittTrigger stepForwardTrigger;
    dsp::SchmittTrigger saveButtonTrigger;
    dsp::SchmittTrigger loadButtonTrigger;
    dsp::SchmittTrigger resetTrigger;
    dsp::PulseGenerator globalTrigger;
    dsp::PulseGenerator globalDisplayTrigger;
    dsp::ClockDivider outputDivider;
    prng::prng prng;
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

        configInput(EXT_SCALE_INPUT, "External scale");
        configInput(STEP_QUEUE_INPUT, "Step: Queue");
        configInput(STEP_TELEPORT_INPUT, "Step: Teleport");
        configInput(STEP_WALK_INPUT, "Step: Walk");
        configInput(STEP_BACK_INPUT, "Step: Back");
        configInput(STEP_FORWARD_INPUT, "Step: Forward");
        configInput(RESET_INPUT, "Reset");

        configOutput(GLOBAL_TRIG_OUTPUT, "Trigger");
        configOutput(GLOBAL_CV_OUTPUT, "1V/Octave pitch");

        for (unsigned i=0; i<NODES; i++) {
            configInput(NODE_SUB_1_SD_INPUT + i, string::f("Step %d: Sub 1", i + 1));
            configInput(NODE_SUB_2_SD_INPUT + i, string::f("Step %d: Sub 2", i + 1));
            configInput(NODE_SUB_3_SD_INPUT + i, string::f("Step %d: Sub 3", i + 1));
            configInput(NODE_SUB_1_OCT_INPUT + i, string::f("Step %d: Sub Oct", i + 1));
            configInput(NODE_ADD_1_SD_INPUT + i, string::f("Step %d: Add 1", i + 1));
            configInput(NODE_ADD_2_SD_INPUT + i, string::f("Step %d: Add 2", i + 1));
            configInput(NODE_ADD_3_SD_INPUT + i, string::f("Step %d: Add 3", i + 1));
            configInput(NODE_ADD_1_OCT_INPUT + i, string::f("Step %d: Add Oct", i + 1));
            configInput(NODE_QUEUE_INPUT + i, string::f("Step %d: Queue", i + 1));

            configOutput(NODE_GATE_OUTPUT + i, string::f("Step %d: Gate", i + 1));
            configOutput(NODE_RANDOM_OUTPUT + i, string::f("Step %d: Random", i + 1));
            configOutput(NODE_LATCH_OUTPUT + i, string::f("Step %d: Latch", i + 1));
            configOutput(NODE_DELAY_OUTPUT + i, string::f("Step %d: Delay", i + 1));
            configOutput(NODE_CV_OUTPUT + i, string::f("Step %d: CV", i + 1));
        }

        configParam(MIN_PARAM, 1.f, 9.f, 3.f, "Minimum Note");
        configParam(MAX_PARAM, 1.f, 9.f, 5.f, "Maximum Note");
        configParam(SLIDE_PARAM, 0.f, 10.f, 0.f, "Slide");
        configParam(TOTAL_NODES_PARAM, 1.f, (float) NODES, (float) NODES, "Total Nodes");
        configParam(QUEUE_CLEAR_MODE_PARAM, 0.f, 1.f, 0.f, "Clear queue after picking from it");
        configParam(REPEAT_MODE_PARAM, 0.f, 1.f, 0.f, "Chance to walk or teleport to the current step");

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

        lcdStatus.layout = Lcd::TEXT1_AND_TEXT2_LAYOUT;
        lcdStatus.text1 = "LEARNING...";
        lcdStatus.text2 = "SUMMONING..";
        lcdStatus.lastInteraction = 0.f;

        prng.init(random::uniform(), random::uniform());
    }

    void onReset() override {
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

        resetDelay = 0.f;
    }

    void randomizePitches() {
        randomizePitchesRequested = false;
        float r = 0.f;
        for (size_t i = 0; i < NODES; i++) {
            r = prng.uniform() * 10.f;
            r = rescale(r, 0.f, 10.f, params[MIN_PARAM].getValue() - 4.f, params[MAX_PARAM].getValue() - 4.f);
            cv[i] = Quantizer::quantize(r, scale);
        }
    }

    void onRandomize() override {
        // Set the MIN/MAX knobs to something reasonable
        params[MIN_PARAM].setValue( prng.uniform() * 2.f + 3.f );
        params[MAX_PARAM].setValue( params[MIN_PARAM].getValue() + prng.uniform() * 2.f + 1.f );

        randomizePitches();
    }

    json_t* dataToJson() override {
        json_t *rootJ = json_object();

        json_object_set_new(rootJ, "currentNode", json_integer(currentNode));

        json_object_set_new(rootJ, "resetStepConfig", json_boolean(resetStepConfig));
        json_object_set_new(rootJ, "resetLoadConfig", json_boolean(resetLoadConfig));
        json_object_set_new(rootJ, "resetQuantizeConfig", json_boolean(resetQuantizeConfig));

        json_t *scaleJ = json_array();
        for (size_t i = 0; i < 12; i++) json_array_insert_new(scaleJ, i, json_boolean(scale[i]));
        json_object_set_new(rootJ, "scale", scaleJ);

        json_t *cvJ = json_array();
        for (size_t i = 0; i < NODES; i++) json_array_insert_new(cvJ, i, json_real(cv[i]));
        json_object_set_new(rootJ, "cv", cvJ);

        json_t *savedCvJ = json_array();
        for (size_t i = 0; i < NODES; i++) json_array_insert_new(savedCvJ, i, json_real(savedCv[i]));
        json_object_set_new(rootJ, "savedCv", savedCvJ);

        json_t *queueJ = json_array();
        for (size_t i = 0; i < NODES; i++) json_array_insert_new(queueJ, i, json_boolean(queue[i]));
        json_object_set_new(rootJ, "queue", queueJ);

        json_t *delayJ = json_array();
        for (size_t i = 0; i < NODES; i++) json_array_insert_new(delayJ, i, json_boolean(delay[i]));
        json_object_set_new(rootJ, "delay", delayJ);

        return rootJ;
    }

    void dataFromJson(json_t* rootJ) override {
        json_t* currentNodeJ = json_object_get(rootJ, "currentNode");
        if (currentNodeJ) currentNode = json_integer_value(currentNodeJ);

        json_t* resetStepConfigJ = json_object_get(rootJ, "resetStepConfig");
        if (resetStepConfigJ) resetStepConfig = json_boolean_value(resetStepConfigJ);

        json_t* resetLoadConfigJ = json_object_get(rootJ, "resetLoadConfig");
        if (resetLoadConfigJ) resetLoadConfig = json_boolean_value(resetLoadConfigJ);

        json_t* resetQuantizeConfigJ = json_object_get(rootJ, "resetQuantizeConfig");
        if (resetQuantizeConfigJ) resetQuantizeConfig = json_boolean_value(resetQuantizeConfigJ);


        json_t *scaleJ = json_object_get(rootJ, "scale");
        if (scaleJ) {
            for (size_t i = 0; i < 12; i++) {
                json_t *scaleNoteJ = json_array_get(scaleJ, i);
                if (scaleNoteJ) scale[i] = json_boolean_value(scaleNoteJ);
            }
        }

        json_t *cvJ = json_object_get(rootJ, "cv");
        if (cvJ) {
            for (size_t i = 0; i < NODES; i++) {
                json_t *cvValueJ = json_array_get(cvJ, i);
                if (cvValueJ) cv[i] = json_real_value(cvValueJ);
            }
        }

        json_t *savedCvJ = json_object_get(rootJ, "savedCv");
        if (savedCvJ) {
            for (size_t i = 0; i < NODES; i++) {
                json_t *savedCvValueJ = json_array_get(savedCvJ, i);
                if (savedCvValueJ) savedCv[i] = json_real_value(savedCvValueJ);
            }
        }

        json_t *queueJ = json_object_get(rootJ, "queue");
        if (queueJ) {
            for (size_t i = 0; i < NODES; i++) {
                json_t *queueStatusJ = json_array_get(queueJ, i);
                if (queueStatusJ) queue[i] = json_boolean_value(queueStatusJ);
            }
        }

        json_t *delayJ = json_object_get(rootJ, "delay");
        if (delayJ) {
            for (size_t i = 0; i < NODES; i++) {
                json_t *delayStatusJ = json_array_get(delayJ, i);
                if (delayStatusJ) delay[i] = json_boolean_value(delayStatusJ);
            }
        }
    }

    void importPortableSequence() {
        pastePortableSequence = false;
        PortableSequence::Sequence sequence;
        sequence.fromClipboard();
        sequence.sort();
        sequence.clampValues();
        size_t max = std::min(NODES, sequence.notes.size());
        for (size_t i = 0; i < max; i++) {
            cv[i] = sequence.notes[i].pitch;
        }
    }

    void exportPortableSequence() {
        copyPortableSequence = false;
        PortableSequence::Sequence sequence;
        PortableSequence::Note note;

        note.length = 1.f;
        sequence.length = 0.f;
        for (size_t i = 0; i < (size_t) params[TOTAL_NODES_PARAM].getValue(); i++){
            note.start = (float) i;
            note.pitch = cv[i];
            sequence.addNote(note);
            sequence.length += 1.f;
        }

        sequence.clampValues();
        sequence.sort();
        sequence.toClipboard();
    }

    void quantizePitches() {
        quantizePitchesRequested = false;
        for (size_t i = 0; i < NODES; i++) cv[i] = Quantizer::quantize(cv[i], scale);
    }

    void processResetInput() {
        resetDelay = 0.f; // This starts the delay
        if(resetLoadConfig) for (size_t i = 0; i < NODES; i++) cv[i] = savedCv[i];
        if(resetStepConfig) currentNode = 0;
        if(resetQuantizeConfig) quantizePitches();
    }

    // True when done waiting
    bool wait1msOnReset(float sampleTime) {
        resetDelay += sampleTime;
        return((resetDelay >= 0.001f) ? true : false);
    }

    void updateScale() {
        if (inputs[EXT_SCALE_INPUT].isConnected() ) {
            for (size_t i = 0; i < 12; i++) scale[i] = (inputs[EXT_SCALE_INPUT].getVoltage(i) > 0.1f) ? true : false;
        } else {
            scale = Quantizer::validNotesInScaleKey( (int) params[SCALE_PARAM].getValue(), (int) params[KEY_PARAM].getValue());
        }
    }

    void updateSlide(){
        slideDuration = params[SLIDE_PARAM].getValue();
        if (slideDuration > 0.00001f ) {
            slideDuration = rescale(slideDuration, 0.f, 10.f, -3.0f, 1.0f);
            slideDuration = powf(10.0f, slideDuration);
        } else {
            slideDuration = 0.f;
        }
    }


    // How many are set by the knob
    size_t getTotalNodes() {
        return (size_t) params[TOTAL_NODES_PARAM].getValue();
    }

    // How many nodes are enqueued
    size_t queueCount() {
        size_t count = 0;
        for(size_t i = 0; i < getTotalNodes(); i++) {
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

    // Does nothing if there's no valid note to jump to
    void subOct(size_t node) {       
        if (cv[node] - 1.f >= getMinCv() - Quantizer::FUDGEOFFSET) {
            // We can remove an octave and stay in bounds
            cv[node] = Quantizer::quantize(cv[node] - 1.f, scale);;
        } else {
            // Separate octave from note
            float nodeOctave = floorf(cv[node]);
            float nodeVoltageOnFirstOctave = cv[node] - nodeOctave;
            float maxOctave = floorf(getMaxCv());
            float candidate = maxOctave + nodeVoltageOnFirstOctave; 
            if (candidate <= getMaxCv() + Quantizer::FUDGEOFFSET && candidate >= getMinCv() - Quantizer::FUDGEOFFSET) {
                // We can wrap around on the max octave
                cv[node] = Quantizer::quantize(candidate, scale);
            } else {
                candidate -= 1.f;
                if (candidate <= getMaxCv() + Quantizer::FUDGEOFFSET && candidate >= getMinCv() - Quantizer::FUDGEOFFSET) {
                    // We can wrap around one octave lower than the max octave
                    cv[node] = Quantizer::quantize(candidate, scale);
                }
            }
        }
    }

    // Does nothing if there's no valid note to jump to
    // Same code as above with + and - and min and max flipped flipways
    void addOct(size_t node) {
        if (cv[node] + 1.f <= getMaxCv() + Quantizer::FUDGEOFFSET) {
            cv[node] = Quantizer::quantize(cv[node] + 1.f, scale);;
        } else {
            float nodeOctave = floorf(cv[node]);
            float nodeVoltageOnFirstOctave = cv[node] - nodeOctave;
            float minOctave = floorf(getMinCv());
            float candidate = minOctave + nodeVoltageOnFirstOctave; 
            if (candidate >= getMinCv() - Quantizer::FUDGEOFFSET && candidate <= getMaxCv() + Quantizer::FUDGEOFFSET) {
                cv[node] = Quantizer::quantize(candidate, scale);
            } else {
                candidate += 1.f;
                if (candidate >= getMinCv() - Quantizer::FUDGEOFFSET && candidate <= getMaxCv() + Quantizer::FUDGEOFFSET) {
                    cv[node] = Quantizer::quantize(candidate, scale);
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
        if(loadButtonTrigger.process(params[LOAD_PARAM].getValue())){
            for (size_t i = 0; i < NODES; i++) cv[i] = savedCv[i];
        }
    }

    void processSaveButton(){
        if(saveButtonTrigger.process(params[SAVE_PARAM].getValue())){
            for (size_t i = 0; i < NODES; i++) savedCv[i] = cv[i];
        }
    }

    // If it's a queue input, something must be already enqueued.
    // Other inputs are accepted without conditions.
    int getStepInput() {
        if (windowTimeout > WINDOWTIMEOUTDURATION) {
            if (stepQueueTrigger.process(inputs[STEP_QUEUE_INPUT].getVoltageSum()) && queueCount() > 0) return STEP_QUEUE;
            if (stepTeleportTrigger.process(inputs[STEP_TELEPORT_INPUT].getVoltageSum()))               return STEP_TELEPORT;
            if (stepWalkTrigger.process(inputs[STEP_WALK_INPUT].getVoltageSum()))                       return STEP_WALK;
            if (stepBackTrigger.process(inputs[STEP_BACK_INPUT].getVoltageSum()))                       return STEP_BACK;
            if (stepForwardTrigger.process(inputs[STEP_FORWARD_INPUT].getVoltageSum()))                 return STEP_FORWARD;
        }
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
            for (size_t i = 0; i < getTotalNodes(); i++) {
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
            if (sub3Sd[i] ) subSd(i, 3);
            if (sub1Oct[i]) subOct(i);
            if (add1Sd[i] ) addSd(i, 1);
            if (add2Sd[i] ) addSd(i, 2);
            if (add3Sd[i] ) addSd(i, 3);
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

        // Teleport never brings back to the current step - unless we only have one,
        // or are in Repeat mode.
        if (stepType == STEP_TELEPORT) {
            if (getTotalNodes() > 1) {
                std::vector<size_t> validNodes;
                for (size_t i = 0; i < getTotalNodes(); i++) {
                    if (params[REPEAT_MODE_PARAM].getValue() == 0.f) {
                        if (i != currentNode) validNodes.push_back(i);
                    } else {
                        validNodes.push_back(i);
                    }
                }
                std::random_shuffle(validNodes.begin(), validNodes.end());
                currentNode = validNodes[0];
            } else {
                currentNode = 0;
            }
        }

        // Random walk can warp around
        if (stepType == STEP_WALK) {
            if (params[REPEAT_MODE_PARAM].getValue() == 1.f && prng.uniform() < 1.f / 3.f) {
                // 1 chance out of 3 the current node repeats
            } else {
                // Then it's a coin flip which direction we go
                if (prng.uniform() >= 0.5f) {
                    // Walk forward
                    if (currentNode >= getTotalNodes() - 1) {
                        currentNode = 0;
                    } else {
                        currentNode++;
                    }
                } else {
                    // Walk back
                    if (currentNode == 0) {
                        currentNode = getTotalNodes() - 1;
                    } else {
                        currentNode--;
                    }
                }
            }
        }

        // Step back can warp around
        if (stepType == STEP_BACK) {
            if (currentNode == 0) {
                currentNode = getTotalNodes() - 1;
            } else {
                currentNode--;
            }
        }

        // Step forward can warp around
        if (stepType == STEP_FORWARD) {
            if (currentNode >= getTotalNodes() - 1) {
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
        lastOutput = cv[currentNode];
        applyTransposes();
        applyQueue();
        applyDelay();
        applyStep();
        updateLatch();
        clearTransposes();
        randomGate = ( prng.uniform() >= 0.5f ) ? true : false;
        globalTrigger.trigger();
        globalDisplayTrigger.trigger(0.003f);
        stepType = -1;
        slideCounter = 0.f;
    }

    // We refresh lotsa stuff, but we don't need to do it at audio rates
    void sendOutputs(const ProcessArgs& args) {
        outputs[GLOBAL_TRIG_OUTPUT].setVoltage( globalTrigger.process(args.sampleTime) ? 10.f : 0.f);
        globalDisplayTrigger.process(args.sampleTime);

        float output = cv[currentNode];
        // Slide
        if (slideDuration > 0.f && slideDuration > slideCounter) {
            output = crossfade(lastOutput, output, (slideCounter / slideDuration) );
            slideCounter += args.sampleTime * OUTPUTDIVIDER;
        }

        outputs[GLOBAL_CV_OUTPUT].setVoltage(output); // TODO: Slide

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

        lcdStatus.processLcd(args.sampleTime);

        if (copyPortableSequence)      exportPortableSequence();
        if (pastePortableSequence)     importPortableSequence();
        if (randomizePitchesRequested) randomizePitches();
        if (quantizePitchesRequested)  quantizePitches();

        // Reset
        if (resetTrigger.process(inputs[RESET_INPUT].getVoltageSum())) processResetInput();
        if (resetDelay >= 0.f) {
            if (wait1msOnReset(args.sampleTime)) {
                // Done with reset
                resetDelay = -1.f;
            } else {
                return;
            }
        }

        if (readWindow < 0.f) {
            // We are not in a Read Window
            stepType = getStepInput();
            if (stepType >= 0) readWindow = 0.f;
            if (windowTimeout < WINDOWTIMEOUTDURATION) windowTimeout += args.sampleTime;
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

        // No need to process this many outputs at audio rates
        if (outputDivider.process()) {
            sendOutputs(args);
            updateScale();
            updateSlide();
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
struct TotalNodesKnob : W::KnobSnap {
    void onDragMove(const event::DragMove& e) override {
        TModule* module = dynamic_cast<TModule*>(getParamQuantity()->module);

        module->lcdStatus.lastInteraction = 0.f;
        module->lcdStatus.dirty = true;
        module->lcdStatus.layout = Lcd::TEXT1_AND_TEXT2_LAYOUT;
        module->lcdStatus.text1 = "";
        module->lcdStatus.text2 = "Nodes: " + std::to_string( (int) module->params[module->TOTAL_NODES_PARAM].getValue());

        W::Knob::onDragMove(e);
    }
};

// Scale/key knobs
template <typename TModule>
struct ScaleKnob : W::KnobSnap {
    void onDragMove(const event::DragMove& e) override {
        TModule* module = dynamic_cast<TModule*>(getParamQuantity()->module);

        module->lcdStatus.lastInteraction = 0.f;
        module->lcdStatus.dirty = true;
        module->lcdStatus.layout = Lcd::PIANO_AND_TEXT2_LAYOUT;

        std::string text = "";
        if (module->params[module->SCALE_PARAM].getValue() == 0.f) {
            text = "CHROMATIC";
        } else {
            text = Quantizer::keyLcdName((int) module->params[module->KEY_PARAM].getValue());
            text.append(" ");
            text.append(Quantizer::scaleLcdName((int) module->params[module->SCALE_PARAM].getValue()));
        }
        if ( module->inputs[module->EXT_SCALE_INPUT].isConnected()) {
            text = "EXTERNAL";
        }
        module->lcdStatus.text2 = text;
        module->lcdStatus.pianoDisplay = module->scale;

        W::Knob::onDragMove(e);
    }
};

// Min/Max knobs
template <typename TModule>
struct MinMaxKnob : W::Knob {
    void onDragMove(const event::DragMove& e) override {
        TModule* module = dynamic_cast<TModule*>(getParamQuantity()->module);

        module->lcdStatus.lastInteraction = 0.f;
        module->lcdStatus.dirty = true;
        module->lcdStatus.layout = Lcd::TEXT1_AND_TEXT2_LAYOUT;
        module->lcdStatus.text1 = "Min: " + Quantizer::noteOctaveLcdName(module->params[module->MIN_PARAM].getValue() - 4.f);
        module->lcdStatus.text2 = "Max: " + Quantizer::noteOctaveLcdName(module->params[module->MAX_PARAM].getValue() - 4.f);

        W::Knob::onDragMove(e);
    }
};

// Slide knobs
template <typename TModule>
struct SlideKnob : W::Knob {
    void onDragMove(const event::DragMove& e) override {
        TModule* module = dynamic_cast<TModule*>(getParamQuantity()->module);

        module->lcdStatus.lastInteraction = 0.f;
        module->lcdStatus.dirty = true;
        module->lcdStatus.layout = Lcd::TEXT1_AND_TEXT2_LAYOUT;
        module->lcdStatus.text1 = "Slide:";

        float displayDuration = module->slideDuration;
        if (displayDuration == 0.f)
            module->lcdStatus.text2 = "DISABLED";
        if (displayDuration > 0.f && displayDuration < 1.f) {
            int displayDurationMs = displayDuration * 1000;
            displayDurationMs = truncf(displayDurationMs);
            module->lcdStatus.text2 = std::to_string(displayDurationMs);
            module->lcdStatus.text2.append("ms");
        } 
        if (displayDuration >= 1.f) {
            module->lcdStatus.text2 = std::to_string(displayDuration);
            module->lcdStatus.text2.resize(4);
            module->lcdStatus.text2.append("s");
        }
        W::Knob::onDragMove(e);
    }
};


// Per-node segment display
template <typename TModule>
struct SegmentDisplay : LightWidget {
	TModule* module;
    size_t node;
	std::shared_ptr<Font> font;
    std::string text = "";

	SegmentDisplay() {
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/dseg/DSEG14ClassicMini-Italic.ttf"));
	}

	void drawLayer(const DrawArgs& args, int layer) override {
		if (layer != 1)
			return;

		nvgFontSize(args.vg, 20);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, 2.0);

        Vec textPos = mm2px(Vec(0.f, 10.f));
		nvgFillColor(args.vg, nvgRGB(0x0b, 0x57, 0x63));
		nvgText(args.vg, textPos.x, textPos.y, "~~~", NULL);
        if (module) {
            if (module->getTotalNodes() > node) {
                nvgFillColor(args.vg, nvgRGB(0xc1, 0xf0, 0xf2));
            } else {
                nvgFillColor(args.vg, nvgRGB(0x76, 0xbf, 0xbe));
            }
            text = Quantizer::noteOctaveSegmentName(module->cv[node]);
            if (node == module->currentNode && module->globalDisplayTrigger.remaining > 0.f) text = "~~~";
            nvgText(args.vg, textPos.x, textPos.y, text.c_str(), NULL);
        }
	}
};

template <typename TModule>
struct SolomonLcdWidget : TransparentWidget {
    TModule *module;
    Lcd::LcdFramebufferWidget<TModule> *lfb;
    Lcd::LcdDrawWidget<TModule> *ldw;

    SolomonLcdWidget(TModule *_module){
        module = _module;
        lfb = new Lcd::LcdFramebufferWidget<TModule>(module);
        ldw = new Lcd::LcdDrawWidget<TModule>(module, "LEARNING...", "SUMMONING..");
        addChild(lfb);
        lfb->addChild(ldw);
    }

    void processDefaultMode() {
        if (!module) return;
        if (module->lcdStatus.lastInteraction != -1.f) return;
        module->lcdStatus.dirty = true;
        module->lcdStatus.layout = Lcd::PIANO_AND_TEXT2_LAYOUT;
        module->lcdStatus.pianoDisplay = Quantizer::pianoDisplay(module->outputs[module->GLOBAL_CV_OUTPUT].getVoltage());
        std::string text = Quantizer::noteOctaveLcdName(module->outputs[module->GLOBAL_CV_OUTPUT].getVoltage());
        text = text + " | " + std::to_string(module->currentNode + 1);
        module->lcdStatus.text2 = text;
    }

    void draw(const DrawArgs& args) override {
        processDefaultMode();
        TransparentWidget::draw(args);
    }
};


template <typename TModule>
struct SegmentDisplayFramebuffer : FramebufferWidget {
    TModule* module;
    size_t node;
    float lastStatus = -20.f;

    void step() override{
        if (module) { 
            if (module->cv[node] != lastStatus || module->globalDisplayTrigger.remaining > 0.f) {
                dirty = true;
            }

            FramebufferWidget::step();
        }
    }
};


// The QUEUE message on the segment display
template <typename TModule>
struct QueueWidget : TransparentWidget {
	TModule* module;
    size_t node;
	FramebufferWidget* framebuffer;
	W::LitSvgWidget* svgWidget;
    bool lastStatus;

    QueueWidget() {
        framebuffer = new FramebufferWidget;
        addChild(framebuffer);
        svgWidget = new W::LitSvgWidget;
        svgWidget->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/solomon-queue-lit.svg")));
        framebuffer->box.size = svgWidget->box.size;
        box.size = svgWidget->box.size;
        framebuffer->addChild(svgWidget);
        lastStatus = true;
    }

    void step() override {
        if(module) {
            if (module->queue[node] != lastStatus) {
                if (module->queue[node] == true) {
                    svgWidget->show();
                } else {
                    svgWidget->hide();
                }
                framebuffer->dirty = true;
            }
            lastStatus = module->queue[node];
        }
        Widget::step();
    }
};


// The DELAY message on the segment display
template <typename TModule>
struct DelayWidget : TransparentWidget {
	TModule* module;
    size_t node;
	FramebufferWidget* framebuffer;
	W::LitSvgWidget* svgWidget;
    bool lastStatus;

    DelayWidget() {
        framebuffer = new FramebufferWidget;
        addChild(framebuffer);
        svgWidget = new W::LitSvgWidget;
        svgWidget->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/solomon-delay-lit.svg")));
        framebuffer->box.size = svgWidget->box.size;
        box.size = svgWidget->box.size;
        framebuffer->addChild(svgWidget);
        lastStatus = true;
    }

    void step() override {
        if(module) {
            if (module->queue[node] != lastStatus) {
                if (module->delay[node] == true) {
                    svgWidget->show();
                } else {
                    svgWidget->hide();
                }
                framebuffer->dirty = true;
            }
            lastStatus = module->delay[node];
        }
        Widget::step();
    }
};


// The PLAY arrow on the segment display
template <typename TModule>
struct PlayWidget : TransparentWidget {
	TModule* module;
    size_t node;
	FramebufferWidget* framebuffer;
	W::LitSvgWidget* svgWidget;
    size_t lastStatus; 

    PlayWidget() {
        framebuffer = new FramebufferWidget;
        addChild(framebuffer);
        svgWidget = new W::LitSvgWidget;
        svgWidget->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/solomon-play-lit.svg")));
        framebuffer->box.size = svgWidget->box.size;
        box.size = svgWidget->box.size;
        framebuffer->addChild(svgWidget);
        lastStatus = true;
    }

    void step() override {
        if(module) {
            if (module->currentNode != lastStatus) {
                if (module->currentNode == node) {
                    svgWidget->show();
                } else {
                    svgWidget->hide();
                }
                framebuffer->dirty = true;
            }
            lastStatus = module->currentNode;
        }
        Widget::step();
    }
};


template <typename TModule>
struct CopyPortableSequenceItem : MenuItem {
    TModule *module;
    void onAction(const event::Action &e) override {
        module->copyPortableSequence = true;
    }
};

template <typename TModule>
struct PastePortableSequenceItem : MenuItem {
    TModule *module;
    void onAction(const event::Action &e) override {
        module->pastePortableSequence = true;
    }
};

template <typename TModule>
struct ResetStepConfigItem : MenuItem {
    TModule *module;
    void onAction(const event::Action &e) override {
        module->resetStepConfig = (module->resetStepConfig) ? false : true;
    }
};

template <typename TModule>
struct ResetLoadConfigItem : MenuItem {
    TModule *module;
    void onAction(const event::Action &e) override {
        module->resetLoadConfig = (module->resetLoadConfig) ? false : true;
    }
};

template <typename TModule>
struct ResetQuantizeConfigItem : MenuItem {
    TModule *module;
    void onAction(const event::Action &e) override {
        module->resetQuantizeConfig = (module->resetQuantizeConfig) ? false : true;
    }
};

template <typename TModule>
struct RandomizePitchesRequestedItem : MenuItem {
    TModule *module;
    void onAction(const event::Action &e) override {
        module->randomizePitchesRequested = true;
    }
};

template <typename TModule>
struct QuantizePitchesRequestedItem : MenuItem {
    TModule *module;
    void onAction(const event::Action &e) override {
        module->quantizePitchesRequested = true;
    }
};


// 8 is the main version, from which the others are copied
struct SolomonWidget8 : W::ModuleWidget {

    SolomonWidget8(Solomon<8>* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/faceplates/Solomon8.svg")));
        
        // Screws
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Signature
        addChild(createWidget<W::Signature>(mm2px(Vec(37.5f, 114.5f))));

        // Queue clear mode
        addParam(createParam<W::RockerSwitchVertical>(mm2px(Vec(28.4f, 17.1f)), module, Solomon<8>::QUEUE_CLEAR_MODE_PARAM));

        // Repeat mode
        addParam(createParam<W::RockerSwitchVertical>(mm2px(Vec(42.4f, 17.1f)), module, Solomon<8>::REPEAT_MODE_PARAM));

        // Global step inputs. Ordered counterclockwise.
        addStaticInput(mm2px(Vec(20.f, 17.f)), module, Solomon<8>::STEP_QUEUE_INPUT);
        addStaticInput(mm2px(Vec( 5.f, 32.f)), module, Solomon<8>::STEP_TELEPORT_INPUT);
        addStaticInput(mm2px(Vec(35.f, 32.f)), module, Solomon<8>::STEP_FORWARD_INPUT);
        addStaticInput(mm2px(Vec(10.f, 47.f)), module, Solomon<8>::STEP_WALK_INPUT);
        addStaticInput(mm2px(Vec(30.f, 47.f)), module, Solomon<8>::STEP_BACK_INPUT);

        // Total Steps
        addParam(createParam<TotalNodesKnob<Solomon<8>>>(mm2px(Vec(20.f, 32.f)), module, Solomon<8>::TOTAL_NODES_PARAM));

        // LCD
        SolomonLcdWidget<Solomon<8>> *lcd = new SolomonLcdWidget<Solomon<8>>(module);
        lcd->box.pos = mm2px(Vec(7.7f, 68.8f));
        addChild(lcd);

        addParam(createParam<ScaleKnob<Solomon<8>>>(mm2px(Vec(15.f, 81.f)), module, Solomon<8>::KEY_PARAM));
        addParam(createParam<ScaleKnob<Solomon<8>>>(mm2px(Vec(27.f, 81.f)), module, Solomon<8>::SCALE_PARAM));
        addStaticInput(mm2px(Vec(39.f, 81.f)), module, Solomon<8>::EXT_SCALE_INPUT);

        addParam(createParam<MinMaxKnob<Solomon<8>>>(mm2px(Vec(15.f, 94.f)), module, Solomon<8>::MIN_PARAM));
        addParam(createParam<MinMaxKnob<Solomon<8>>>(mm2px(Vec(27.f, 94.f)), module, Solomon<8>::MAX_PARAM));
        addParam(createParam<SlideKnob<Solomon<8>>>(mm2px(Vec(39.f, 94.f)), module, Solomon<8>::SLIDE_PARAM));

        // Reset
        addStaticInput(mm2px(Vec(3.f, 107.f)), module, Solomon<8>::RESET_INPUT);

        // Global output
        addStaticOutput(mm2px(Vec(15.f, 107.f)), module, Solomon<8>::GLOBAL_TRIG_OUTPUT);
        addStaticOutput(mm2px(Vec(27.f, 107.f)), module, Solomon<8>::GLOBAL_CV_OUTPUT);

        // Load and Save
        addParam(createParam<W::ButtonMomentary>(mm2px(Vec(3.f, 81.f)), module, Solomon<8>::SAVE_PARAM));
        addParam(createParam<W::ButtonMomentary>(mm2px(Vec(3.f, 94.f)), module, Solomon<8>::LOAD_PARAM));

        // Nodes
        float xOffset = 53.f;
        float yOffset = 17.f;
        for(size_t i = 0; i < 8; i++) {
            // Inputs
            addStaticInput(mm2px(Vec(xOffset +  5.f, yOffset +  0.f)), module, Solomon<8>::NODE_QUEUE_INPUT     + i);
            addStaticInput(mm2px(Vec(xOffset +  0.f, yOffset + 10.f)), module, Solomon<8>::NODE_SUB_1_OCT_INPUT + i);
            addStaticInput(mm2px(Vec(xOffset +  0.f, yOffset + 20.f)), module, Solomon<8>::NODE_SUB_3_SD_INPUT  + i);
            addStaticInput(mm2px(Vec(xOffset +  0.f, yOffset + 30.f)), module, Solomon<8>::NODE_SUB_2_SD_INPUT  + i);
            addStaticInput(mm2px(Vec(xOffset +  0.f, yOffset + 40.f)), module, Solomon<8>::NODE_SUB_1_SD_INPUT  + i);
            addStaticInput(mm2px(Vec(xOffset + 10.f, yOffset + 10.f)), module, Solomon<8>::NODE_ADD_1_OCT_INPUT + i);
            addStaticInput(mm2px(Vec(xOffset + 10.f, yOffset + 20.f)), module, Solomon<8>::NODE_ADD_3_SD_INPUT  + i);
            addStaticInput(mm2px(Vec(xOffset + 10.f, yOffset + 30.f)), module, Solomon<8>::NODE_ADD_2_SD_INPUT  + i);
            addStaticInput(mm2px(Vec(xOffset + 10.f, yOffset + 40.f)), module, Solomon<8>::NODE_ADD_1_SD_INPUT  + i);

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
            addParam(createParam<W::ButtonMomentary>(mm2px(Vec(xOffset +  0.f, yOffset + 64.f)), module, Solomon<8>::NODE_SUB_1_SD_PARAM + i));
            addParam(createParam<W::ButtonMomentary>(mm2px(Vec(xOffset + 10.f, yOffset + 64.f)), module, Solomon<8>::NODE_ADD_1_SD_PARAM + i));
            addParam(createParam<W::ButtonMomentary>(mm2px(Vec(xOffset +  5.f, yOffset + 71.f)), module, Solomon<8>::NODE_QUEUE_PARAM + i));

            // Outputs
            addStaticOutput(mm2px(Vec(xOffset +  0.f, yOffset +  80.f)), module, Solomon<8>::NODE_GATE_OUTPUT + i);
            addStaticOutput(mm2px(Vec(xOffset + 10.f, yOffset +  80.f)), module, Solomon<8>::NODE_RANDOM_OUTPUT  + i);
            addStaticOutput(mm2px(Vec(xOffset +  5.f, yOffset +  88.f)), module, Solomon<8>::NODE_CV_OUTPUT + i);
            addStaticOutput(mm2px(Vec(xOffset +  0.f, yOffset +  96.f)), module, Solomon<8>::NODE_LATCH_OUTPUT   + i);
            addStaticOutput(mm2px(Vec(xOffset + 10.f, yOffset +  96.f)), module, Solomon<8>::NODE_DELAY_OUTPUT    + i);

            xOffset += 25.f;
        }
    }

    void appendContextMenu(ui::Menu *menu) override {	
        Solomon<8> *module = dynamic_cast<Solomon<8>*>(this->module);
        assert(module);

        menu->addChild(new MenuSeparator());

        CopyPortableSequenceItem<Solomon<8>> *copyPortableSequenceItem = createMenuItem<CopyPortableSequenceItem<Solomon<8>>>("Copy Portable Sequence");
        copyPortableSequenceItem->module = module;
        menu->addChild(copyPortableSequenceItem);

        PastePortableSequenceItem<Solomon<8>> *pastePortableSequenceItem = createMenuItem<PastePortableSequenceItem<Solomon<8>>>("Paste Portable Sequence");
        pastePortableSequenceItem->module = module;
        menu->addChild(pastePortableSequenceItem);

        menu->addChild(new MenuSeparator());

        ResetStepConfigItem<Solomon<8>> *resetStepConfigItem = createMenuItem<ResetStepConfigItem<Solomon<8>>>("Reset input goes back to first step");
        resetStepConfigItem->module = module;
        resetStepConfigItem->rightText += (module->resetStepConfig) ? "✔" : "";
        menu->addChild(resetStepConfigItem);

        ResetLoadConfigItem<Solomon<8>> *resetLoadConfigItem = createMenuItem<ResetLoadConfigItem<Solomon<8>>>("Reset input loads the saved pattern");
        resetLoadConfigItem->module = module;
        resetLoadConfigItem->rightText += (module->resetLoadConfig) ? "✔" : "";
        menu->addChild(resetLoadConfigItem);

        ResetQuantizeConfigItem<Solomon<8>> *resetQuantizeConfigItem = createMenuItem<ResetQuantizeConfigItem<Solomon<8>>>("Reset input quantizes the pattern");
        resetQuantizeConfigItem->module = module;
        resetQuantizeConfigItem->rightText += (module->resetQuantizeConfig) ? "✔" : "";
        menu->addChild(resetQuantizeConfigItem);

        menu->addChild(new MenuSeparator());

        RandomizePitchesRequestedItem<Solomon<8>> *randomizePitchesRequestedItem = createMenuItem<RandomizePitchesRequestedItem<Solomon<8>>>("Randomize all nodes");
        randomizePitchesRequestedItem->module = module;
        menu->addChild(randomizePitchesRequestedItem);

        QuantizePitchesRequestedItem<Solomon<8>> *quantizePitchesRequestedItem = createMenuItem<QuantizePitchesRequestedItem<Solomon<8>>>("Quantize all nodes");
        quantizePitchesRequestedItem->module = module;
        menu->addChild(quantizePitchesRequestedItem);
    }

};





///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////




struct SolomonWidget4 : W::ModuleWidget {

    SolomonWidget4(Solomon<4>* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/faceplates/Solomon4.svg")));
        
        // Screws
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Signature
        addChild(createWidget<W::Signature>(mm2px(Vec(37.5f, 114.5f))));

        // Queue clear mode
        addParam(createParam<W::RockerSwitchVertical>(mm2px(Vec(28.4f, 17.1f)), module, Solomon<4>::QUEUE_CLEAR_MODE_PARAM));

        // Repeat mode
        addParam(createParam<W::RockerSwitchVertical>(mm2px(Vec(42.4f, 17.1f)), module, Solomon<4>::REPEAT_MODE_PARAM));

        // Global step inputs. Ordered counterclockwise.
        addStaticInput(mm2px(Vec(20.f, 17.f)), module, Solomon<4>::STEP_QUEUE_INPUT);
        addStaticInput(mm2px(Vec( 5.f, 32.f)), module, Solomon<4>::STEP_TELEPORT_INPUT);
        addStaticInput(mm2px(Vec(35.f, 32.f)), module, Solomon<4>::STEP_FORWARD_INPUT);
        addStaticInput(mm2px(Vec(10.f, 47.f)), module, Solomon<4>::STEP_WALK_INPUT);
        addStaticInput(mm2px(Vec(30.f, 47.f)), module, Solomon<4>::STEP_BACK_INPUT);

        // Total Steps
        addParam(createParam<TotalNodesKnob<Solomon<4>>>(mm2px(Vec(20.f, 32.f)), module, Solomon<4>::TOTAL_NODES_PARAM));

        // LCD
        SolomonLcdWidget<Solomon<4>> *lcd = new SolomonLcdWidget<Solomon<4>>(module);
        lcd->box.pos = mm2px(Vec(7.7f, 68.8f));
        addChild(lcd);

        addParam(createParam<ScaleKnob<Solomon<4>>>(mm2px(Vec(15.f, 81.f)), module, Solomon<4>::KEY_PARAM));
        addParam(createParam<ScaleKnob<Solomon<4>>>(mm2px(Vec(27.f, 81.f)), module, Solomon<4>::SCALE_PARAM));
        addStaticInput(mm2px(Vec(39.f, 81.f)), module, Solomon<4>::EXT_SCALE_INPUT);

        addParam(createParam<MinMaxKnob<Solomon<4>>>(mm2px(Vec(15.f, 94.f)), module, Solomon<4>::MIN_PARAM));
        addParam(createParam<MinMaxKnob<Solomon<4>>>(mm2px(Vec(27.f, 94.f)), module, Solomon<4>::MAX_PARAM));
        addParam(createParam<SlideKnob<Solomon<4>>>(mm2px(Vec(39.f, 94.f)), module, Solomon<4>::SLIDE_PARAM));

        // Reset
        addStaticInput(mm2px(Vec(3.f, 107.f)), module, Solomon<4>::RESET_INPUT);

        // Global output
        addStaticOutput(mm2px(Vec(15.f, 107.f)), module, Solomon<4>::GLOBAL_TRIG_OUTPUT);
        addStaticOutput(mm2px(Vec(27.f, 107.f)), module, Solomon<4>::GLOBAL_CV_OUTPUT);

        // Load and Save
        addParam(createParam<W::ButtonMomentary>(mm2px(Vec(3.f, 81.f)), module, Solomon<4>::SAVE_PARAM));
        addParam(createParam<W::ButtonMomentary>(mm2px(Vec(3.f, 94.f)), module, Solomon<4>::LOAD_PARAM));

        // Nodes
        float xOffset = 53.f;
        float yOffset = 17.f;
        for(size_t i = 0; i < 4; i++) {
            // Inputs
            addStaticInput(mm2px(Vec(xOffset +  5.f, yOffset +  0.f)), module, Solomon<4>::NODE_QUEUE_INPUT     + i);
            addStaticInput(mm2px(Vec(xOffset +  0.f, yOffset + 10.f)), module, Solomon<4>::NODE_SUB_1_OCT_INPUT + i);
            addStaticInput(mm2px(Vec(xOffset +  0.f, yOffset + 20.f)), module, Solomon<4>::NODE_SUB_3_SD_INPUT  + i);
            addStaticInput(mm2px(Vec(xOffset +  0.f, yOffset + 30.f)), module, Solomon<4>::NODE_SUB_2_SD_INPUT  + i);
            addStaticInput(mm2px(Vec(xOffset +  0.f, yOffset + 40.f)), module, Solomon<4>::NODE_SUB_1_SD_INPUT  + i);
            addStaticInput(mm2px(Vec(xOffset + 10.f, yOffset + 10.f)), module, Solomon<4>::NODE_ADD_1_OCT_INPUT + i);
            addStaticInput(mm2px(Vec(xOffset + 10.f, yOffset + 20.f)), module, Solomon<4>::NODE_ADD_3_SD_INPUT  + i);
            addStaticInput(mm2px(Vec(xOffset + 10.f, yOffset + 30.f)), module, Solomon<4>::NODE_ADD_2_SD_INPUT  + i);
            addStaticInput(mm2px(Vec(xOffset + 10.f, yOffset + 40.f)), module, Solomon<4>::NODE_ADD_1_SD_INPUT  + i);

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
            addParam(createParam<W::ButtonMomentary>(mm2px(Vec(xOffset +  0.f, yOffset + 64.f)), module, Solomon<4>::NODE_SUB_1_SD_PARAM + i));
            addParam(createParam<W::ButtonMomentary>(mm2px(Vec(xOffset + 10.f, yOffset + 64.f)), module, Solomon<4>::NODE_ADD_1_SD_PARAM + i));
            addParam(createParam<W::ButtonMomentary>(mm2px(Vec(xOffset +  5.f, yOffset + 71.f)), module, Solomon<4>::NODE_QUEUE_PARAM + i));

            // Outputs
            addStaticOutput(mm2px(Vec(xOffset +  0.f, yOffset +  80.f)), module, Solomon<4>::NODE_GATE_OUTPUT + i);
            addStaticOutput(mm2px(Vec(xOffset + 10.f, yOffset +  80.f)), module, Solomon<4>::NODE_RANDOM_OUTPUT  + i);
            addStaticOutput(mm2px(Vec(xOffset +  5.f, yOffset +  88.f)), module, Solomon<4>::NODE_CV_OUTPUT + i);
            addStaticOutput(mm2px(Vec(xOffset +  0.f, yOffset +  96.f)), module, Solomon<4>::NODE_LATCH_OUTPUT   + i);
            addStaticOutput(mm2px(Vec(xOffset + 10.f, yOffset +  96.f)), module, Solomon<4>::NODE_DELAY_OUTPUT    + i);

            xOffset += 25.f;
        }
    }

    void appendContextMenu(ui::Menu *menu) override {	
        Solomon<4> *module = dynamic_cast<Solomon<4>*>(this->module);
        assert(module);

        menu->addChild(new MenuSeparator());

        CopyPortableSequenceItem<Solomon<4>> *copyPortableSequenceItem = createMenuItem<CopyPortableSequenceItem<Solomon<4>>>("Copy Portable Sequence");
        copyPortableSequenceItem->module = module;
        menu->addChild(copyPortableSequenceItem);

        PastePortableSequenceItem<Solomon<4>> *pastePortableSequenceItem = createMenuItem<PastePortableSequenceItem<Solomon<4>>>("Paste Portable Sequence");
        pastePortableSequenceItem->module = module;
        menu->addChild(pastePortableSequenceItem);

        menu->addChild(new MenuSeparator());

        ResetStepConfigItem<Solomon<4>> *resetStepConfigItem = createMenuItem<ResetStepConfigItem<Solomon<4>>>("Reset input goes back to first step");
        resetStepConfigItem->module = module;
        resetStepConfigItem->rightText += (module->resetStepConfig) ? "✔" : "";
        menu->addChild(resetStepConfigItem);

        ResetLoadConfigItem<Solomon<4>> *resetLoadConfigItem = createMenuItem<ResetLoadConfigItem<Solomon<4>>>("Reset input loads the saved pattern");
        resetLoadConfigItem->module = module;
        resetLoadConfigItem->rightText += (module->resetLoadConfig) ? "✔" : "";
        menu->addChild(resetLoadConfigItem);

        ResetQuantizeConfigItem<Solomon<4>> *resetQuantizeConfigItem = createMenuItem<ResetQuantizeConfigItem<Solomon<4>>>("Reset input quantizes the pattern");
        resetQuantizeConfigItem->module = module;
        resetQuantizeConfigItem->rightText += (module->resetQuantizeConfig) ? "✔" : "";
        menu->addChild(resetQuantizeConfigItem);

        menu->addChild(new MenuSeparator());

        RandomizePitchesRequestedItem<Solomon<4>> *randomizePitchesRequestedItem = createMenuItem<RandomizePitchesRequestedItem<Solomon<4>>>("Randomize all nodes");
        randomizePitchesRequestedItem->module = module;
        menu->addChild(randomizePitchesRequestedItem);

        QuantizePitchesRequestedItem<Solomon<4>> *quantizePitchesRequestedItem = createMenuItem<QuantizePitchesRequestedItem<Solomon<4>>>("Quantize all nodes");
        quantizePitchesRequestedItem->module = module;
        menu->addChild(quantizePitchesRequestedItem);
    }

};




///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////



struct SolomonWidget16 : W::ModuleWidget {

    SolomonWidget16(Solomon<16>* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/faceplates/Solomon16.svg")));
        
        // Screws
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Signature
        addChild(createWidget<W::Signature>(mm2px(Vec(37.5f, 114.5f))));

        // Queue clear mode
        addParam(createParam<W::RockerSwitchVertical>(mm2px(Vec(28.4f, 17.1f)), module, Solomon<16>::QUEUE_CLEAR_MODE_PARAM));

        // Repeat mode
        addParam(createParam<W::RockerSwitchVertical>(mm2px(Vec(42.4f, 17.1f)), module, Solomon<16>::REPEAT_MODE_PARAM));

        // Global step inputs. Ordered counterclockwise.
        addStaticInput(mm2px(Vec(20.f, 17.f)), module, Solomon<16>::STEP_QUEUE_INPUT);
        addStaticInput(mm2px(Vec( 5.f, 32.f)), module, Solomon<16>::STEP_TELEPORT_INPUT);
        addStaticInput(mm2px(Vec(35.f, 32.f)), module, Solomon<16>::STEP_FORWARD_INPUT);
        addStaticInput(mm2px(Vec(10.f, 47.f)), module, Solomon<16>::STEP_WALK_INPUT);
        addStaticInput(mm2px(Vec(30.f, 47.f)), module, Solomon<16>::STEP_BACK_INPUT);

        // Total Steps
        addParam(createParam<TotalNodesKnob<Solomon<16>>>(mm2px(Vec(20.f, 32.f)), module, Solomon<16>::TOTAL_NODES_PARAM));

        // LCD
        SolomonLcdWidget<Solomon<16>> *lcd = new SolomonLcdWidget<Solomon<16>>(module);
        lcd->box.pos = mm2px(Vec(7.7f, 68.8f));
        addChild(lcd);

        addParam(createParam<ScaleKnob<Solomon<16>>>(mm2px(Vec(15.f, 81.f)), module, Solomon<16>::KEY_PARAM));
        addParam(createParam<ScaleKnob<Solomon<16>>>(mm2px(Vec(27.f, 81.f)), module, Solomon<16>::SCALE_PARAM));
        addStaticInput(mm2px(Vec(39.f, 81.f)), module, Solomon<16>::EXT_SCALE_INPUT);

        addParam(createParam<MinMaxKnob<Solomon<16>>>(mm2px(Vec(15.f, 94.f)), module, Solomon<16>::MIN_PARAM));
        addParam(createParam<MinMaxKnob<Solomon<16>>>(mm2px(Vec(27.f, 94.f)), module, Solomon<16>::MAX_PARAM));
        addParam(createParam<SlideKnob<Solomon<16>>>(mm2px(Vec(39.f, 94.f)), module, Solomon<16>::SLIDE_PARAM));

        // Reset
        addStaticInput(mm2px(Vec(3.f, 107.f)), module, Solomon<16>::RESET_INPUT);

        // Global output
        addStaticOutput(mm2px(Vec(15.f, 107.f)), module, Solomon<16>::GLOBAL_TRIG_OUTPUT);
        addStaticOutput(mm2px(Vec(27.f, 107.f)), module, Solomon<16>::GLOBAL_CV_OUTPUT);

        // Load and Save
        addParam(createParam<W::ButtonMomentary>(mm2px(Vec(3.f, 81.f)), module, Solomon<16>::SAVE_PARAM));
        addParam(createParam<W::ButtonMomentary>(mm2px(Vec(3.f, 94.f)), module, Solomon<16>::LOAD_PARAM));

        // Nodes
        float xOffset = 53.f;
        float yOffset = 17.f;
        for(size_t i = 0; i < 16; i++) {
            // Inputs
            addStaticInput(mm2px(Vec(xOffset +  5.f, yOffset +  0.f)), module, Solomon<16>::NODE_QUEUE_INPUT     + i);
            addStaticInput(mm2px(Vec(xOffset +  0.f, yOffset + 10.f)), module, Solomon<16>::NODE_SUB_1_OCT_INPUT + i);
            addStaticInput(mm2px(Vec(xOffset +  0.f, yOffset + 20.f)), module, Solomon<16>::NODE_SUB_3_SD_INPUT  + i);
            addStaticInput(mm2px(Vec(xOffset +  0.f, yOffset + 30.f)), module, Solomon<16>::NODE_SUB_2_SD_INPUT  + i);
            addStaticInput(mm2px(Vec(xOffset +  0.f, yOffset + 40.f)), module, Solomon<16>::NODE_SUB_1_SD_INPUT  + i);
            addStaticInput(mm2px(Vec(xOffset + 10.f, yOffset + 10.f)), module, Solomon<16>::NODE_ADD_1_OCT_INPUT + i);
            addStaticInput(mm2px(Vec(xOffset + 10.f, yOffset + 20.f)), module, Solomon<16>::NODE_ADD_3_SD_INPUT  + i);
            addStaticInput(mm2px(Vec(xOffset + 10.f, yOffset + 30.f)), module, Solomon<16>::NODE_ADD_2_SD_INPUT  + i);
            addStaticInput(mm2px(Vec(xOffset + 10.f, yOffset + 40.f)), module, Solomon<16>::NODE_ADD_1_SD_INPUT  + i);

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
            addParam(createParam<W::ButtonMomentary>(mm2px(Vec(xOffset +  0.f, yOffset + 64.f)), module, Solomon<16>::NODE_SUB_1_SD_PARAM + i));
            addParam(createParam<W::ButtonMomentary>(mm2px(Vec(xOffset + 10.f, yOffset + 64.f)), module, Solomon<16>::NODE_ADD_1_SD_PARAM + i));
            addParam(createParam<W::ButtonMomentary>(mm2px(Vec(xOffset +  5.f, yOffset + 71.f)), module, Solomon<16>::NODE_QUEUE_PARAM + i));

            // Outputs
            addStaticOutput(mm2px(Vec(xOffset +  0.f, yOffset +  80.f)), module, Solomon<16>::NODE_GATE_OUTPUT + i);
            addStaticOutput(mm2px(Vec(xOffset + 10.f, yOffset +  80.f)), module, Solomon<16>::NODE_RANDOM_OUTPUT  + i);
            addStaticOutput(mm2px(Vec(xOffset +  5.f, yOffset +  88.f)), module, Solomon<16>::NODE_CV_OUTPUT + i);
            addStaticOutput(mm2px(Vec(xOffset +  0.f, yOffset +  96.f)), module, Solomon<16>::NODE_LATCH_OUTPUT   + i);
            addStaticOutput(mm2px(Vec(xOffset + 10.f, yOffset +  96.f)), module, Solomon<16>::NODE_DELAY_OUTPUT    + i);

            xOffset += 25.f;
        }
    }

    void appendContextMenu(ui::Menu *menu) override {	
        Solomon<16> *module = dynamic_cast<Solomon<16>*>(this->module);
        assert(module);

        menu->addChild(new MenuSeparator());

        CopyPortableSequenceItem<Solomon<16>> *copyPortableSequenceItem = createMenuItem<CopyPortableSequenceItem<Solomon<16>>>("Copy Portable Sequence");
        copyPortableSequenceItem->module = module;
        menu->addChild(copyPortableSequenceItem);

        PastePortableSequenceItem<Solomon<16>> *pastePortableSequenceItem = createMenuItem<PastePortableSequenceItem<Solomon<16>>>("Paste Portable Sequence");
        pastePortableSequenceItem->module = module;
        menu->addChild(pastePortableSequenceItem);

        menu->addChild(new MenuSeparator());

        ResetStepConfigItem<Solomon<16>> *resetStepConfigItem = createMenuItem<ResetStepConfigItem<Solomon<16>>>("Reset input goes back to first step");
        resetStepConfigItem->module = module;
        resetStepConfigItem->rightText += (module->resetStepConfig) ? "✔" : "";
        menu->addChild(resetStepConfigItem);

        ResetLoadConfigItem<Solomon<16>> *resetLoadConfigItem = createMenuItem<ResetLoadConfigItem<Solomon<16>>>("Reset input loads the saved pattern");
        resetLoadConfigItem->module = module;
        resetLoadConfigItem->rightText += (module->resetLoadConfig) ? "✔" : "";
        menu->addChild(resetLoadConfigItem);

        ResetQuantizeConfigItem<Solomon<16>> *resetQuantizeConfigItem = createMenuItem<ResetQuantizeConfigItem<Solomon<16>>>("Reset input quantizes the pattern");
        resetQuantizeConfigItem->module = module;
        resetQuantizeConfigItem->rightText += (module->resetQuantizeConfig) ? "✔" : "";
        menu->addChild(resetQuantizeConfigItem);

        menu->addChild(new MenuSeparator());

        RandomizePitchesRequestedItem<Solomon<16>> *randomizePitchesRequestedItem = createMenuItem<RandomizePitchesRequestedItem<Solomon<16>>>("Randomize all nodes");
        randomizePitchesRequestedItem->module = module;
        menu->addChild(randomizePitchesRequestedItem);

        QuantizePitchesRequestedItem<Solomon<16>> *quantizePitchesRequestedItem = createMenuItem<QuantizePitchesRequestedItem<Solomon<16>>>("Quantize all nodes");
        quantizePitchesRequestedItem->module = module;
        menu->addChild(quantizePitchesRequestedItem);
    }

};


} // Namespace Solomon

Model* modelSolomon4 = createModel<Solomon::Solomon<4>, Solomon::SolomonWidget4>("Solomon4");
Model* modelSolomon8 = createModel<Solomon::Solomon<8>, Solomon::SolomonWidget8>("Solomon8");
Model* modelSolomon16 = createModel<Solomon::Solomon<16>, Solomon::SolomonWidget16>("Solomon16");
