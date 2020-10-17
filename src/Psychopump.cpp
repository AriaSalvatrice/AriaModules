/*  Copyright (C) 2019-2020 Aria Salvatrice
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/
#include <queue>
#include "plugin.hpp"
#include "quantizer.hpp"
#include "lcd.hpp"

namespace Psychopump {

// Lots of values are hardcoded in this module, but amount of channels is the most common magic number.
// I might make a smaller version of this module if it's popular, but for now only one version.
const size_t CHANNELS = 8;

// TODO: Make gates longer
// TODO: Retrigger
// TODO: CV
// TODO: Pitch

// Contains one of everything I want to handle polyphonically.
struct OutputChannel {
    std::array<float, 8> cv;
    std::array<float, 2> pt;
    float pitch = 0.f;
    std::array<std::queue<bool>, 2> gateQueue;

    OutputChannel() {
        for (size_t i = 0; i < 8; i++) cv[i] = 0.f;
        for (size_t i = 0; i < 2; i++) pt[i] = 0.f;
    }

    // This must be called by the module when delay gets disabled.
    void emptyQueue() {
        std::array<std::queue<bool>, 2> empty;
        std::swap(gateQueue, empty);
    }

    // This must be called exactly once per getGateVoltage()
    void setGate(bool status, size_t outputNumber) {
        if (status) {
            gateQueue[outputNumber].push(true);
        } else {
            gateQueue[outputNumber].push(false);
        }
    }

    // This must be called exactly once per setGate(), even if the value is not necessary.
    float getGateVoltage(bool delayEnabled, size_t outputNumber) {
        float r;
        if (delayEnabled && gateQueue[outputNumber].size() < 5) {
            // Delay enabled recently - wait for it to fill
            r = 0.f;
        } else {
            // Queue full or delay disabled (queue was cleared)
            r = (gateQueue[outputNumber].front()) ? 10.f : 0.f;
            gateQueue[outputNumber].pop();
        }
        return r;
    }

};



// Keeping track of enums across two dimensions would be a huge pain, so instead I'm going
// with a lot of duplication. The rule: if it's vertical (a channel) on the module, it's enumerated.
// If it's horizontal (a parm or input), it's named. 

struct Psychopump : Module {
    enum ParamIds {
        ENUMS(CV0_PARAM, CHANNELS),
        ENUMS(CV1_PARAM, CHANNELS),
        ENUMS(CV2_PARAM, CHANNELS),
        ENUMS(CV3_PARAM, CHANNELS),
        ENUMS(CV4_PARAM, CHANNELS),
        ENUMS(CV5_PARAM, CHANNELS),
        ENUMS(CV6_PARAM, CHANNELS),
        ENUMS(CV7_PARAM, CHANNELS),
        ENUMS(GATE_LENGTH_PARAM, CHANNELS),
        ENUMS(CV0_PLUS_RANDOM_PARAM, CHANNELS),
        ENUMS(CV1_PLUS_RANDOM_PARAM, CHANNELS),
        ENUMS(CV2_PLUS_RANDOM_PARAM, CHANNELS),
        ENUMS(CV3_PLUS_RANDOM_PARAM, CHANNELS),
        ENUMS(CV4_PLUS_RANDOM_PARAM, CHANNELS),
        ENUMS(CV5_PLUS_RANDOM_PARAM, CHANNELS),
        ENUMS(CV6_PLUS_RANDOM_PARAM, CHANNELS),
        ENUMS(CV7_PLUS_RANDOM_PARAM, CHANNELS),
        ENUMS(CV0_MINUS_RANDOM_PARAM, CHANNELS),
        ENUMS(CV1_MINUS_RANDOM_PARAM, CHANNELS),
        ENUMS(CV2_MINUS_RANDOM_PARAM, CHANNELS),
        ENUMS(CV3_MINUS_RANDOM_PARAM, CHANNELS),
        ENUMS(CV4_MINUS_RANDOM_PARAM, CHANNELS),
        ENUMS(CV5_MINUS_RANDOM_PARAM, CHANNELS),
        ENUMS(CV6_MINUS_RANDOM_PARAM, CHANNELS),
        ENUMS(CV7_MINUS_RANDOM_PARAM, CHANNELS),
        ENUMS(CV_RANDOM_OFFSET_PARAM, CHANNELS),
        ENUMS(CV_POLARITY_PARAM, CHANNELS),
        ENUMS(PT1_PLUS_RANDOM_PARAM, CHANNELS),
        ENUMS(PT2_PLUS_RANDOM_PARAM, CHANNELS),
        ENUMS(PT1_MINUS_RANDOM_PARAM, CHANNELS),
        ENUMS(PT2_MINUS_RANDOM_PARAM, CHANNELS),
        PT1_RANDOM_OFFSET_PARAM,
        PT2_RANDOM_OFFSET_PARAM,
        ENUMS(PITCH_PLUS_RANDOM_PARAM, CHANNELS),
        ENUMS(PITCH_MINUS_RANDOM_PARAM, CHANNELS),
        PITCH_RANDOM_OFFSET_PARAM,
        ENUMS(CHANNEL_LABEL_PARAM, CHANNELS),
        ENUMS(OUTPUT_LABEL_PARAM, CHANNELS),
        ENUMS(MUTE_PARAM, CHANNELS),
        ENUMS(SOLO_PARAM, CHANNELS),
        ENUMS(OUT1_PARAM, CHANNELS),
        ENUMS(OUT2_PARAM, CHANNELS),
        ENUMS(RANDOMIZE_PARAM, CHANNELS),
        ENUMS(QUANTIZE_PARAM, CHANNELS),
        POLY_MODE_PARAM,
        GATE_DELAY_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        ENUMS(GATE_INPUT, CHANNELS),
        ENUMS(PT1_INPUT, CHANNELS),
        ENUMS(PT2_INPUT, CHANNELS),
        ENUMS(PITCH_INPUT, CHANNELS),
        POLY_EXTERNAL_SCALE_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        CV0_OUTPUT,
        CV1_OUTPUT,
        CV2_OUTPUT,
        CV3_OUTPUT,
        CV4_OUTPUT,
        CV5_OUTPUT,
        CV6_OUTPUT,
        CV7_OUTPUT,
        PT1_OUTPUT,
        PT2_OUTPUT,
        GATE1_OUTPUT,
        GATE2_OUTPUT,
        PITCH_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        ENUMS(ROW_LIGHT, CHANNELS),
        POLY_MODE_MONO_LIGHT,
        POLY_MODE_ROTATE_LIGHT,
        POLY_MODE_ALL_LIGHT,
        GATE1_LIGHT,
        GATE2_LIGHT,
        NUM_LIGHTS
    };
    enum PolyModes {
        MONO_MODE,
        ROTATE_MODE,
        ALL_MODE,
        NUM_POLYPHONY_MODES
    };

    Lcd::LcdStatus lcdStatus;
    dsp::Timer processTimer;
    dsp::SchmittTrigger polyButtonTrigger;
    std::array<std::string, CHANNELS> rowLabels;
    std::array<std::string, CHANNELS> columnLabels;
    std::array<OutputChannel, CHANNELS> outputChannels;
    std::array<size_t, CHANNELS> channelOrder;
    std::array<std::array<bool, CHANNELS>, 2> gates;
    size_t polyMode = MONO_MODE;
    bool delayEnabled = false;
    bool lastDelayStatus = false;
    
    Psychopump() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        std::string label;
        for(size_t i = 0; i < CHANNELS; i++) {
            channelOrder[i] = i;
            gates[0][i] = false;
            gates[1][i] = false;

            label = "Knob 1 - Channel ";
            label.append(std::to_string(i + 1));
            configParam(CV0_PARAM + i, 0.f, 10.f, 5.f, label, "V");
            label = "Knob 2 - Channel ";
            label.append(std::to_string(i + 1));
            configParam(CV1_PARAM + i, 0.f, 10.f, 5.f, label, "V");
            label = "Knob 3 - Channel ";
            label.append(std::to_string(i + 1));
            configParam(CV2_PARAM + i, 0.f, 10.f, 5.f, label, "V");
            label = "Knob 4 - Channel ";
            label.append(std::to_string(i + 1));
            configParam(CV3_PARAM + i, 0.f, 10.f, 5.f, label, "V");
            label = "Knob 5 - Channel ";
            label.append(std::to_string(i + 1));
            configParam(CV4_PARAM + i, 0.f, 10.f, 5.f, label, "V");
            label = "Knob 6 - Channel ";
            label.append(std::to_string(i + 1));
            configParam(CV5_PARAM + i, 0.f, 10.f, 5.f, label, "V");
            label = "Knob 7 - Channel ";
            label.append(std::to_string(i + 1));
            configParam(CV6_PARAM + i, 0.f, 10.f, 5.f, label, "V");
            label = "Knob 8 - Channel ";
            label.append(std::to_string(i + 1));
            configParam(CV7_PARAM + i, 0.f, 10.f, 5.f, label, "V");

            label = "Gate ";
            label.append(std::to_string(i + 1));
            rowLabels[i] = label;

            label = "Knob ";
            label.append(std::to_string(i + 1));
            columnLabels[i] = label;

            configParam(CV0_PLUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Add random offset");
            configParam(CV1_PLUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Add random offset");
            configParam(CV2_PLUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Add random offset");
            configParam(CV3_PLUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Add random offset");
            configParam(CV4_PLUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Add random offset");
            configParam(CV5_PLUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Add random offset");
            configParam(CV6_PLUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Add random offset");
            configParam(CV7_PLUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Add random offset");
            configParam(PT1_PLUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Add random offset");
            configParam(PT2_PLUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Add random offset");
            configParam(PITCH_PLUS_RANDOM_PARAM + i, 0.f, 1.f, 0.f, "Add random offset");

            configParam(CV0_MINUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Subtract random offset");
            configParam(CV1_MINUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Subtract random offset");
            configParam(CV2_MINUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Subtract random offset");
            configParam(CV3_MINUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Subtract random offset");
            configParam(CV4_MINUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Subtract random offset");
            configParam(CV5_MINUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Subtract random offset");
            configParam(CV6_MINUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Subtract random offset");
            configParam(CV7_MINUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Subtract random offset");
            configParam(PT1_MINUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Subtract random offset");
            configParam(PT2_MINUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Subtract random offset");
            configParam(PITCH_MINUS_RANDOM_PARAM + i, 0.f, 1.f, 0.f, "Subtract random offset");
            
            configParam(GATE_LENGTH_PARAM + i, 0.f, 1.f, 0.f, "Change Gate length");

            configParam(MUTE_PARAM + i, 0.f, 1.f, 0.f, "Mute");
            configParam(SOLO_PARAM + i, 0.f, 1.f, 0.f, "Solo");
            configParam(OUT1_PARAM + i, 0.f, 1.f, 1.f, "Gate to Output 1");
            configParam(OUT2_PARAM + i, 0.f, 1.f, 0.f, "Gate to Output 2");

            label = "Randomize knobs on Channel ";
            label.append(std::to_string(i + 1));
            configParam(RANDOMIZE_PARAM + i, 0.f, 1.f, 0.f, label);

            configParam(QUANTIZE_PARAM + i, 0.f, 2.f, 0.f, "Quantize");

            configParam(CHANNEL_LABEL_PARAM + i, 0.f, 1.f, 0.f, "Add Channel label on LCD");
            configParam(OUTPUT_LABEL_PARAM + i, 0.f, 1.f, 0.f, "Add Output label on LCD");

            label = "Random offset for Knob ";
            label.append(std::to_string(i + 1));
            configParam(CV_RANDOM_OFFSET_PARAM + i, 0.f, 5.f, 0.f, label, "V");
            configParam(CV_POLARITY_PARAM + i, 0.f, 1.f, 0.f, "Unipolar / Bipolar");
        }
        configParam(PT1_RANDOM_OFFSET_PARAM, 0.f, 5.f, 0.f, "Random offset for Pass-through 1");
        configParam(PT2_RANDOM_OFFSET_PARAM, 0.f, 5.f, 0.f, "Random offset for Pass-through 2");
        configParam(PITCH_RANDOM_OFFSET_PARAM, 0.f, 12.f, 0.f, "Random offset for Pitch", " semitones");
        configParam(POLY_MODE_PARAM, 0.f, 1.f, 0.f, "Polyphony Mode");
        configParam(GATE_DELAY_PARAM, 0.f, 1.f, 0.f, "5ms gate delay for modules that snapshot CV on trigger");

        lcdStatus.text1 = "Insert Obol";
        lcdStatus.text2 = " To Depart";
        lcdStatus.layout = Lcd::TEXT1_AND_TEXT2_LAYOUT;
    }

    // TODO: More logic on mode change here.
    void updatePolyphonyMode() {
        if (polyButtonTrigger.process(params[POLY_MODE_PARAM].getValue())) {
            polyMode++;
            if (polyMode == NUM_POLYPHONY_MODES) polyMode = MONO_MODE;
        }
    }

    size_t getMaxPolyphonyChannels() {
        if (polyMode == ROTATE_MODE) {
            return 3;
        }
        if (polyMode == ALL_MODE) {
            for (size_t i = CHANNELS; i > 0; i--) {
                if (inputs[GATE_INPUT + i - 1].isConnected()) return i;
            }
        }
        // If monophonic, or polyphonic but nothing plugged in
        return 1;
    }

    // Empties every channel's delay queue if the delay was just disabled
    void updateDelayStatus() {
        delayEnabled = (params[GATE_DELAY_PARAM].getValue() == 1.f) ? true : false;
        if (!delayEnabled && delayEnabled != lastDelayStatus) {
            for(size_t i = 0; i < CHANNELS; i++) outputChannels[i].emptyQueue();
        }
        lastDelayStatus = delayEnabled;
    }

    // Pushes a gate to the delay queue, popped by updateGates()
    void updateTriggers() {
        for (size_t i = 0; i < CHANNELS; i++) {
            outputChannels[i].setGate((inputs[GATE_INPUT + i].getVoltageSum() > 0.1f), 0);
            outputChannels[i].setGate((inputs[GATE_INPUT + i].getVoltageSum() > 0.1f), 1);
        }
    }
    
    // This also pops the delay queue.
    void updateGates() {
        for (size_t i = 0; i < CHANNELS; i++) {
            gates[0][i] = outputChannels[i].getGateVoltage(delayEnabled, 0);
            gates[1][i] = outputChannels[i].getGateVoltage(delayEnabled, 1);
        }
    }

    void sendOutput() {
        // TODO: Handle polyphony.
        size_t max = getMaxPolyphonyChannels();

        for (size_t i = 0; i < max; i++) {
            outputs[CV0_OUTPUT].setVoltage(outputChannels[ channelOrder[i] ].cv[0], i);
            outputs[CV1_OUTPUT].setVoltage(outputChannels[ channelOrder[i] ].cv[1], i);
            outputs[CV2_OUTPUT].setVoltage(outputChannels[ channelOrder[i] ].cv[2], i);
            outputs[CV3_OUTPUT].setVoltage(outputChannels[ channelOrder[i] ].cv[3], i);
            outputs[CV4_OUTPUT].setVoltage(outputChannels[ channelOrder[i] ].cv[4], i);
            outputs[CV5_OUTPUT].setVoltage(outputChannels[ channelOrder[i] ].cv[5], i);
            outputs[CV6_OUTPUT].setVoltage(outputChannels[ channelOrder[i] ].cv[6], i);
            outputs[CV7_OUTPUT].setVoltage(outputChannels[ channelOrder[i] ].cv[7], i);
            outputs[PT1_OUTPUT].setVoltage(outputChannels[ channelOrder[i] ].pt[0], i);
            outputs[PT2_OUTPUT].setVoltage(outputChannels[ channelOrder[i] ].pt[1], i);
            outputs[PITCH_OUTPUT].setVoltage(outputChannels[ channelOrder[i] ].pitch, i);
            outputs[GATE1_OUTPUT].setVoltage((gates[0][ channelOrder[i] ] ? 10.f : 0.f), i);
            outputs[GATE2_OUTPUT].setVoltage((gates[1][ channelOrder[i] ] ? 10.f : 0.f), i);
        }
        outputs[CV0_OUTPUT].setChannels(max);
        outputs[CV1_OUTPUT].setChannels(max);
        outputs[CV2_OUTPUT].setChannels(max);
        outputs[CV3_OUTPUT].setChannels(max);
        outputs[CV4_OUTPUT].setChannels(max);
        outputs[CV5_OUTPUT].setChannels(max);
        outputs[CV6_OUTPUT].setChannels(max);
        outputs[CV7_OUTPUT].setChannels(max);
        outputs[PT1_OUTPUT].setChannels(max);
        outputs[PT2_OUTPUT].setChannels(max);
        outputs[PITCH_OUTPUT].setChannels(max);
        outputs[GATE1_OUTPUT].setChannels(max);
        outputs[GATE2_OUTPUT].setChannels(max);
    }

    void updateLights() {
        lights[POLY_MODE_MONO_LIGHT].setBrightness((polyMode == MONO_MODE) ? 1.f : 0.f);
        lights[POLY_MODE_ROTATE_LIGHT].setBrightness((polyMode == ROTATE_MODE) ? 1.f : 0.f);
        lights[POLY_MODE_ALL_LIGHT].setBrightness((polyMode == ALL_MODE) ? 1.f : 0.f);
    }
    
    void process(const ProcessArgs& args) override {
        // We want the module to run at 1hz or below, regardless of sample rate, so that gates and
        // silences are always at least 1ms.
        // https://community.vcvrack.com/t/what-is-the-shortest-safe-gap-in-between-gates-to-retrigger/11303
        if (processTimer.process(args.sampleTime) > 0.001f) {
            processTimer.reset();
            updatePolyphonyMode();
            updateDelayStatus();
            updateTriggers();
            updateGates();
            sendOutput();
            updateLights();
            // TODO: Channel order
        }
    }
};




// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------



// Thank you https://github.com/stoermelder/vcvrack-packone/blob/v1/src/Glue.cpp 
// for figuring out how to autofocus a field properly lol 
struct ChannelLabelField : TextField {
    Psychopump* module;
    size_t row = 0;

    // Force close on Enter
    void onSelectKey(const event::SelectKey& e) override {
        if (e.action == GLFW_PRESS && e.key == GLFW_KEY_ENTER) {
            module->rowLabels[row] = text;
            getAncestorOfType<ui::MenuOverlay>()->requestDelete();
            e.consume(this);
        }
        if (!e.getTarget()) {
            TextField::onSelectKey(e);
        }
    }

    // Auto-focus and auto-save
    void step() override {
        APP->event->setSelected(this);
        module->rowLabels[row] = text;
        TextField::step();
    }
};

struct OutputLabelField : TextField {
    Psychopump* module;
    size_t column = 0;

    void onSelectKey(const event::SelectKey& e) override {
        if (e.action == GLFW_PRESS && e.key == GLFW_KEY_ENTER) {
            module->columnLabels[column] = text;
            getAncestorOfType<ui::MenuOverlay>()->requestDelete();
            e.consume(this);
        }
        if (!e.getTarget()) {
            TextField::onSelectKey(e);
        }
    }

    void step() override {
        APP->event->setSelected(this);
        module->columnLabels[column] = text;
        TextField::step();
    }
};

struct GateLabelButton : W::LitSvgSwitchUnshadowed {
    Psychopump* module;
    size_t row = 0;
    GateLabelButton() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/label-button-right-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/label-button-right-on.svg")));
        momentary = true;
    }
    void onButton(const event::Button& e) override {
    	if (e.button != GLFW_MOUSE_BUTTON_LEFT) return; // Skip context menu
        ui::Menu* menu = createMenu();
        menu->addChild(createMenuLabel("Channel label on LCD:"));
        ChannelLabelField* channelLabelField = new ChannelLabelField;
        channelLabelField->module = module;
        channelLabelField->row = row;
        channelLabelField->box.size.x = 80.f;
        channelLabelField->placeholder = "Label";
        channelLabelField->setText(module->rowLabels[row]);
        channelLabelField->selectAll();
        menu->addChild(channelLabelField);
        e.consume(this);
    }
};

struct OutputLabelButton : W::LitSvgSwitchUnshadowed {
    Psychopump* module;
    size_t column = 0;
    OutputLabelButton() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/label-button-bottom-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/label-button-bottom-on.svg")));
        momentary = true;
    }
    void onButton(const event::Button& e) override {
    	if (e.button != GLFW_MOUSE_BUTTON_LEFT) return; // Skip context menu
        ui::Menu* menu = createMenu();
        menu->addChild(createMenuLabel("Output label on LCD:"));
        OutputLabelField* outputLabelField = new OutputLabelField;
        outputLabelField->module = module;
        outputLabelField->column = column;
        outputLabelField->box.size.x = 80.f;
        outputLabelField->placeholder = "Label";
        outputLabelField->setText(module->columnLabels[column]);
        outputLabelField->selectAll();
        menu->addChild(outputLabelField);
        e.consume(this);
    }
};


struct FortuneButton : W::LitSvgSwitchUnshadowed {
    FortuneButton() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/fortune-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/fortune-on.svg")));
        momentary = true;
    }
};

struct QuantizeButton : W::LitSvgSwitchUnshadowed {
    QuantizeButton() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/quantize-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/quantize-on.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/quantize-pink.svg")));
    }
};

struct PlusButton : W::LitSvgSwitchUnshadowed {
    PlusButton() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pmbutton-plus-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pmbutton-plus-on.svg")));
    }
};

struct MinusButton : W::LitSvgSwitchUnshadowed {
    MinusButton() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pmbutton-minus-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pmbutton-minus-on.svg")));
    }
};

struct MuteButton : W::LitSvgSwitchUnshadowed {
    MuteButton() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/psychopump-mute-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/psychopump-mute-on.svg")));
    }
};

struct SoloButton : W::LitSvgSwitchUnshadowed {
    SoloButton() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/psychopump-solo-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/psychopump-solo-on.svg")));
    }
};

struct Out1Button : W::LitSvgSwitchUnshadowed {
    Out1Button() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/psychopump-out1-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/psychopump-out1-on.svg")));
    }
};

struct Out2Button : W::LitSvgSwitchUnshadowed {
    Out2Button() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/psychopump-out2-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/psychopump-out2-on.svg")));
    }
};

struct RockerSwitchUB : W::SvgSwitchUnshadowed {
    RockerSwitchUB() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/rocker-switch-ub-u.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/rocker-switch-ub-b.svg")));
    }
};



// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------



struct PsychopumpWidget : W::ModuleWidget {

    void addGateInputs(float xOffset, float yOffset, Psychopump* module) {
        for(size_t i = 0; i < 8; i++) {
            GateLabelButton* gateLabelButton = new GateLabelButton;
            gateLabelButton->box.pos = mm2px(Vec(xOffset + 4.1f, yOffset + i * 10.f));
            gateLabelButton->row = i;
            if (module) {
                gateLabelButton->module = module;
                gateLabelButton->paramQuantity = module->paramQuantities[Psychopump::CHANNEL_LABEL_PARAM + i];
            }
            addParam(gateLabelButton);
            addStaticInput(mm2px(Vec(xOffset, yOffset + i * 10.f)), module, Psychopump::GATE_INPUT + i);
        }
    }

    void addChannelControls(float xOffset, float yOffset, Psychopump* module) {
        for(size_t i = 0; i < 8; i++) {
            addParam(createParam<MuteButton>(mm2px(Vec(xOffset +  0.f, yOffset +  0.f + i * 10.f)), module, Psychopump::MUTE_PARAM + i));
            addParam(createParam<SoloButton>(mm2px(Vec(xOffset +  0.f, yOffset + 4.1f + i * 10.f)), module, Psychopump::SOLO_PARAM + i));
            addParam(createParam<Out1Button>(mm2px(Vec(xOffset + 5.2f, yOffset +  0.f + i * 10.f)), module, Psychopump::OUT1_PARAM + i));
            addParam(createParam<Out2Button>(mm2px(Vec(xOffset + 5.2f, yOffset + 4.1f + i * 10.f)), module, Psychopump::OUT2_PARAM + i));
        }
    }

    void addPassThroughInputs(float xOffset, float yOffset, Psychopump* module) {
        for(size_t i = 0; i < 8; i++) {
            addParam(createParam<PlusButton>(mm2px(Vec(xOffset + 4.1f, i * 10.f + yOffset)), module, Psychopump::PT1_PLUS_RANDOM_PARAM + i));
            addParam(createParam<MinusButton>(mm2px(Vec(xOffset + 4.1f, i * 10.f + yOffset + 3.95f)), module, Psychopump::PT1_MINUS_RANDOM_PARAM + i));
            addParam(createParam<PlusButton>(mm2px(Vec(xOffset + 4.1f + 14.f, i * 10.f + yOffset)), module, Psychopump::PT2_PLUS_RANDOM_PARAM + i));
            addParam(createParam<MinusButton>(mm2px(Vec(xOffset + 4.1f + 14.f, i * 10.f + yOffset + 3.95f)), module, Psychopump::PT2_MINUS_RANDOM_PARAM + i));
            addStaticInput(mm2px(Vec( 0.f + xOffset,  i * 10.f + yOffset)), module, Psychopump::PT1_INPUT + i);
            addStaticInput(mm2px(Vec(14.f + xOffset,  i * 10.f + yOffset)), module, Psychopump::PT2_INPUT + i);
        }
    }

    void addPassThroughRandomOffsetKnobs(float xOffset, float yOffset, Psychopump* module) {
        addParam(createParam<W::Knob>(mm2px(Vec(xOffset,        yOffset)), module, Psychopump::PT1_RANDOM_OFFSET_PARAM));
        addParam(createParam<W::Knob>(mm2px(Vec(xOffset + 14.f, yOffset)), module, Psychopump::PT2_RANDOM_OFFSET_PARAM));
    }

    void addPassThroughOutputs(float xOffset, float yOffset, Psychopump* module) {
        addStaticOutput(mm2px(Vec(xOffset, yOffset)), module, Psychopump::PT1_OUTPUT);
        addStaticOutput(mm2px(Vec(xOffset + 14.f, yOffset)), module, Psychopump::PT2_OUTPUT);
    }

    void addPassThroughSection(float xOffset, float yOffset, Psychopump* module) {
        addPassThroughInputs(xOffset, yOffset, module);
        addPassThroughRandomOffsetKnobs(xOffset + 2.9f, yOffset + 80.f, module);
        addPassThroughOutputs(xOffset + 2.9f, yOffset + 90.f, module);
    }

    void addRandomizeButtons(float xOffset, float yOffset, Psychopump* module) {
        for(size_t i = 0; i < 8; i++) {
            addParam(createParam<FortuneButton>(mm2px(Vec(xOffset, yOffset + 1.5f + i * 10.f)), module, Psychopump::RANDOMIZE_PARAM + i));
        }
    }

    void addCVParamElement(float xOffset, float yOffset, Psychopump* module, int light, int cvParam, int plusRandomParam, int minusRandomParam) {
        addParam(createParam<PlusButton>(mm2px(Vec(xOffset + 4.1f, yOffset)), module, plusRandomParam));
        addParam(createParam<MinusButton>(mm2px(Vec(xOffset + 4.1f, yOffset + 3.95f)), module, minusRandomParam));
        addChild(W::createKnobLight<W::KnobLightYellow>(mm2px(Vec(xOffset, yOffset)), module, light, cvParam, 0.f, 10.f));
        addParam(createParam<W::KnobTransparent>(mm2px(Vec(xOffset, yOffset)), module, cvParam));
    }

    void addCVParams(float xOffset, float yOffset, Psychopump* module) {
        for(size_t i = 0; i < 8; i++) {
            addCVParamElement( 0 * 14.f + xOffset, i * 10.f + yOffset, module, Psychopump::ROW_LIGHT + i, Psychopump::CV0_PARAM + i, Psychopump::CV0_PLUS_RANDOM_PARAM + i, Psychopump::CV0_MINUS_RANDOM_PARAM + i);
            addCVParamElement( 1 * 14.f + xOffset, i * 10.f + yOffset, module, Psychopump::ROW_LIGHT + i, Psychopump::CV1_PARAM + i, Psychopump::CV1_PLUS_RANDOM_PARAM + i, Psychopump::CV1_MINUS_RANDOM_PARAM + i);
            addCVParamElement( 2 * 14.f + xOffset, i * 10.f + yOffset, module, Psychopump::ROW_LIGHT + i, Psychopump::CV2_PARAM + i, Psychopump::CV2_PLUS_RANDOM_PARAM + i, Psychopump::CV2_MINUS_RANDOM_PARAM + i);
            addCVParamElement( 3 * 14.f + xOffset, i * 10.f + yOffset, module, Psychopump::ROW_LIGHT + i, Psychopump::CV3_PARAM + i, Psychopump::CV3_PLUS_RANDOM_PARAM + i, Psychopump::CV3_MINUS_RANDOM_PARAM + i);
            addCVParamElement( 4 * 14.f + xOffset, i * 10.f + yOffset, module, Psychopump::ROW_LIGHT + i, Psychopump::CV4_PARAM + i, Psychopump::CV4_PLUS_RANDOM_PARAM + i, Psychopump::CV4_MINUS_RANDOM_PARAM + i);
            addCVParamElement( 5 * 14.f + xOffset, i * 10.f + yOffset, module, Psychopump::ROW_LIGHT + i, Psychopump::CV5_PARAM + i, Psychopump::CV5_PLUS_RANDOM_PARAM + i, Psychopump::CV5_MINUS_RANDOM_PARAM + i);
            addCVParamElement( 6 * 14.f + xOffset, i * 10.f + yOffset, module, Psychopump::ROW_LIGHT + i, Psychopump::CV6_PARAM + i, Psychopump::CV6_PLUS_RANDOM_PARAM + i, Psychopump::CV6_MINUS_RANDOM_PARAM + i);
            addCVParamElement( 7 * 14.f + xOffset, i * 10.f + yOffset, module, Psychopump::ROW_LIGHT + i, Psychopump::CV7_PARAM + i, Psychopump::CV7_PLUS_RANDOM_PARAM + i, Psychopump::CV7_MINUS_RANDOM_PARAM + i);
        }
    }

    void addCVPolaritySwitches(float xOffset, float yOffset, Psychopump* module) {
        for(size_t i = 0; i < 8; i++) {
            addParam(createParam<RockerSwitchUB>(mm2px(Vec(xOffset + i * 14.f, yOffset)), module, Psychopump::CV_POLARITY_PARAM + i));
        }
    }

    void addCVRandomOffsetKnobs(float xOffset, float yOffset, Psychopump* module) {
        for(size_t i = 0; i < 8; i++) {
            addParam(createParam<W::Knob>(mm2px(Vec(xOffset + i * 14.f, yOffset)), module, Psychopump::CV_RANDOM_OFFSET_PARAM + i));
        }
    }

    void addCVOutputs(float xOffset, float yOffset, Psychopump* module) {
        for(size_t i = 0; i < 8; i++) {
            OutputLabelButton* outputLabelButton = new OutputLabelButton;
            outputLabelButton->box.pos = mm2px(Vec(xOffset + i * 14.f, yOffset + 4.1f));
            outputLabelButton->column = i;
            if (module) {
                outputLabelButton->module = module;
                outputLabelButton->paramQuantity = module->paramQuantities[Psychopump::OUTPUT_LABEL_PARAM + i];
            }
            addParam(outputLabelButton);
        }

        addStaticOutput(mm2px(Vec(xOffset + 0 * 14.f, yOffset)), module, Psychopump::CV0_OUTPUT);
        addStaticOutput(mm2px(Vec(xOffset + 1 * 14.f, yOffset)), module, Psychopump::CV1_OUTPUT);
        addStaticOutput(mm2px(Vec(xOffset + 2 * 14.f, yOffset)), module, Psychopump::CV2_OUTPUT);
        addStaticOutput(mm2px(Vec(xOffset + 3 * 14.f, yOffset)), module, Psychopump::CV3_OUTPUT);
        addStaticOutput(mm2px(Vec(xOffset + 4 * 14.f, yOffset)), module, Psychopump::CV4_OUTPUT);
        addStaticOutput(mm2px(Vec(xOffset + 5 * 14.f, yOffset)), module, Psychopump::CV5_OUTPUT);
        addStaticOutput(mm2px(Vec(xOffset + 6 * 14.f, yOffset)), module, Psychopump::CV6_OUTPUT);
        addStaticOutput(mm2px(Vec(xOffset + 7 * 14.f, yOffset)), module, Psychopump::CV7_OUTPUT);
    }

    void addCVParamsSection(float xOffset, float yOffset, Psychopump* module) {
        addCVParams(xOffset, yOffset, module); // 112mm
        addCVPolaritySwitches(xOffset + 3.f, yOffset - 4.8f, module);
        addCVRandomOffsetKnobs(xOffset + 2.9f, yOffset + 80.f, module);
        addCVOutputs(xOffset + 2.9f, yOffset + 90.f, module); // + 2.9mm
    }

    void addQuantizeButtons(float xOffset, float yOffset, Psychopump* module) {
        for(size_t i = 0; i < 8; i++) {
            addParam(createParam<QuantizeButton>(mm2px(Vec(xOffset, yOffset + 1.5f + i * 10.f)), module, Psychopump::QUANTIZE_PARAM + i));
        }
    }

    void addPitchInputs(float xOffset, float yOffset, Psychopump* module) {
        for(size_t i = 0; i < 8; i++) {
            addParam(createParam<PlusButton>(mm2px(Vec(xOffset + 4.1f, i * 10.f + yOffset)), module, Psychopump::PITCH_PLUS_RANDOM_PARAM + i));
            addParam(createParam<MinusButton>(mm2px(Vec(xOffset + 4.1f, i * 10.f + yOffset + 3.95f)), module, Psychopump::PITCH_MINUS_RANDOM_PARAM + i));
            addStaticInput(mm2px(Vec( 0.f + xOffset,  i * 10.f + yOffset)), module, Psychopump::PITCH_INPUT + i);
        }
    }

    void addPitchSection(float xOffset, float yOffset, Psychopump* module) {
        addPitchInputs(xOffset, yOffset, module);
        addParam(createParam<W::KnobSnap>(mm2px(Vec(xOffset + 2.9f, yOffset + 80.f)), module, Psychopump::PITCH_RANDOM_OFFSET_PARAM));
        addStaticOutput(mm2px(Vec(xOffset + 2.9f, yOffset + 90.f)), module, Psychopump::PITCH_OUTPUT);
    }

    void addGateLengthControls(float xOffset, float yOffset, Psychopump* module) {
        for(size_t i = 0; i < 8; i++) {
            addParam(createParam<W::Knob>(mm2px(Vec(xOffset, yOffset + i * 10.f)), module, Psychopump::GATE_LENGTH_PARAM + i));
        }
    }


    PsychopumpWidget(Psychopump* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/faceplates/Psychopump.svg")));
        
        // Screws
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Signature
        addChild(createWidget<W::Signature>(mm2px(Vec(237.0f, 114.5f))));

        // Main Area
        addGateInputs(5.f, 24.f, module); // ~14mm
        addChannelControls(19.f, 24.f, module);  // ~10mm
        addGateLengthControls(31.f, 24.f, module); // 10mm
        addPassThroughSection(40.f, 24.f, module); // 28mm
        addRandomizeButtons(68.1f, 24.f, module); // ~7mm
        addCVParamsSection(75.f, 24.f, module); // 112mm
        addQuantizeButtons(187.1f, 24.f, module); // ~7mm
        addPitchSection(194.f, 24.f, module); // 14mm

        // Gate outputs
        addDynamicOutput(mm2px(Vec(13.2f, 114.f)), module, Psychopump::GATE1_OUTPUT, Psychopump::GATE1_LIGHT);
        addDynamicOutput(mm2px(Vec(27.2f, 114.f)), module, Psychopump::GATE2_OUTPUT, Psychopump::GATE2_LIGHT);

        // LCD
        Lcd::LcdWidget<Psychopump> *lcd = new Lcd::LcdWidget<Psychopump>(module, "Insert Obol", " To Depart");
        lcd->box.pos = mm2px(Vec(214.6f, 28.1f));
        addChild(lcd);

        // PES
        addStaticInput(mm2px(Vec(213.f, 44.f)), module, Psychopump::POLY_EXTERNAL_SCALE_INPUT);

        // Polyphony
        addParam(createParam<W::ButtonMomentary>(mm2px(Vec(213.f, 54.f)), module, Psychopump::POLY_MODE_PARAM));
        addChild(createLight<W::StatusLightInput>(mm2px(Vec(223.f, 54.f)), module, Psychopump::POLY_MODE_MONO_LIGHT));
        addChild(createLight<W::StatusLightInput>(mm2px(Vec(223.f, 57.5f)), module, Psychopump::POLY_MODE_ROTATE_LIGHT));
        addChild(createLight<W::StatusLightInput>(mm2px(Vec(223.f, 61.f)), module, Psychopump::POLY_MODE_ALL_LIGHT));

        // Delay
        addParam(createParam<W::Button>(mm2px(Vec(213.f, 64.f)), module, Psychopump::GATE_DELAY_PARAM));

    }
};

} // namespace Psychopump

Model* modelPsychopump = createModel<Psychopump::Psychopump, Psychopump::PsychopumpWidget>("Psychopump");
