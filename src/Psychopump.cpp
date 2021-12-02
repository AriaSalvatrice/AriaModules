/*  Copyright (C) 2019-2020 Aria Salvatrice
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/
#include <deque>
#include <queue>
#include "plugin.hpp"
#include "quantizer.hpp"
#include "lcd.hpp"


namespace Psychopump {

// Lots of values are hardcoded in this module, but amount of channels is the most common magic number.
//It will help if I want to make a smaller version of this module one day. But it will only happen if it's popular.
const size_t CHANNELS = 8;

// Contains everything that can be handled polyphonically.
// Also handles delay and retriggering. Only gates are delayed.
// Each matches directly a gate input: which one(s) to actually output is the module's choice.
struct OutputChannel {
    std::array<float, 8> cv;
    std::array<float, 2> sh;
    float pitch = 0.f;
    std::array<std::queue<bool>, 2> gateQueue;
    std::array<bool, 2> gateStatus;
    // set in ms as soon as it's enqueued
    std::array<int, 2> gateSustain;
    // true only once it's started
    std::array<bool, 2> gateSustaining;
    std::array<bool, 2> retriggered;
    // Reset by the lcd
    std::array<bool, 2> gateDisplay;


    OutputChannel() {
        reset();
    }


    void reset() {
        for (size_t i = 0; i < 8; i++) cv[i] = 0.f;
        for (size_t i = 0; i < 2; i++) sh[i] = 0.f;
        pitch = 0.f;
        for (size_t i = 0; i < 2; i++) gateStatus[i] = false;
        emptyQueue();
    }


    // This must be called explicitly by the module when delay gets enabled or disabled.
    void emptyQueue() {
        std::array<std::queue<bool>, 2> empty;
        std::swap(gateQueue, empty);
        for (size_t i = 0; i < 2; i++) gateSustain[i] = 0;
        for (size_t i = 0; i < 2; i++) gateSustaining[i] = false;
        for (size_t i = 0; i < 2; i++) retriggered[i] = false;
        for (size_t i = 0; i < 2; i++) gateDisplay[i] = false;
    }


    void clampValues() {
        for (size_t i = 0; i < 8; i++) clamp(cv[i], -10.f, 10.f);
        for (size_t i = 0; i < 2; i++) clamp(sh[i], -10.f, 10.f);
        clamp(pitch, -10.f, 10.f);
    }


    // Sustain in ms, 0 if disabled.
    // When gate length change is enabled, updateGate should only receive the initial trigger once, not the full gate.
    void updateGate(bool status, size_t sustain, bool delayEnabled, size_t outputNumber) {
        // If delay is not enabled, the FIFO queue holds only that one value, which is read and popped later.
        gateQueue[outputNumber].push( (retriggered[outputNumber]) ? true : status);

        if (delayEnabled && gateQueue[outputNumber].size() < 5) {
            // Delay was enabled less than 5ms ago: no processing, fill up the delay first.
            gateStatus[outputNumber] = false;
            return;
        }
        
        if (gateSustaining[outputNumber]) {
            if (status) {
                // On retrigger, silence now, and enqueue a retrigger next step. (If delay enabled, silence is longer)
                gateStatus[outputNumber] = false;
                retriggered[outputNumber] = true;
                gateSustaining[outputNumber] = false;
                gateSustain[outputNumber] = sustain;
            } else {
                // No retrigger
                gateStatus[outputNumber] = true;
                gateSustain[outputNumber]--;
                if (gateSustain[outputNumber] <= 0) gateSustaining[outputNumber] = false;
            }
        } else {
            // There's no gate still sustaining, so read from the queue
            gateStatus[outputNumber] = gateQueue[outputNumber].front();
            // Enqueue sustain if any
            if (status && sustain > 1) gateSustain[outputNumber] = sustain - 1;
            // Only start sustaining if we're outputting a gate
            if (gateStatus[outputNumber] && gateSustain[outputNumber] > 0) gateSustaining[outputNumber] = true;
            retriggered[outputNumber] = false;
        }

        // Set by false by the LCD once displayed. Ensure trigs are seen for at least a frame.
        if (gateStatus[outputNumber]) gateDisplay[outputNumber] = true;

        // Keep the queue the length we found it in
        gateQueue[outputNumber].pop();
    }


    float getGateVoltage(size_t outputNumber) {
        return (gateStatus[outputNumber]) ? 10.f : 0.f;
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
        ENUMS(SH0_PLUS_RANDOM_PARAM, CHANNELS),
        ENUMS(SH1_PLUS_RANDOM_PARAM, CHANNELS),
        ENUMS(SH0_MINUS_RANDOM_PARAM, CHANNELS),
        ENUMS(SH1_MINUS_RANDOM_PARAM, CHANNELS),
        SH0_RANDOM_OFFSET_PARAM,
        SH1_RANDOM_OFFSET_PARAM,
        ENUMS(PITCH_PLUS_RANDOM_PARAM, CHANNELS),
        ENUMS(PITCH_MINUS_RANDOM_PARAM, CHANNELS),
        PITCH_RANDOM_OFFSET_PARAM,
        ENUMS(CHANNEL_LABEL_PARAM, CHANNELS),
        ENUMS(OUTPUT_LABEL_PARAM, CHANNELS),
        ENUMS(MUTE_PARAM, CHANNELS),
        ENUMS(SOLO_PARAM, CHANNELS),
        ENUMS(OUT0_PARAM, CHANNELS),
        ENUMS(OUT1_PARAM, CHANNELS),
        ENUMS(RANDOMIZE_PARAM, CHANNELS),
        ENUMS(QUANTIZE_PARAM, CHANNELS),
        POLY_MODE_PARAM,
        GATE_DELAY_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        ENUMS(GATE_INPUT, CHANNELS),
        ENUMS(SH0_INPUT, CHANNELS),
        ENUMS(SH1_INPUT, CHANNELS),
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
        SH0_OUTPUT,
        SH1_OUTPUT,
        GATE0_OUTPUT,
        GATE1_OUTPUT,
        PITCH_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        ENUMS(CHANNEL_LIGHT, CHANNELS),
        POLY_MODE_MONO_LIGHT,
        POLY_MODE_ROTATE_LIGHT,
        POLY_MODE_ALL_LIGHT,
        GATE0_LIGHT,
        GATE1_LIGHT,
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
    dsp::SchmittTrigger randomizeCvTrigger[8];
    std::array<std::string, CHANNELS> channelLabels;
    std::array<std::string, CHANNELS> outputLabels;
    std::array<OutputChannel, CHANNELS> outputChannels;
    std::array<size_t, CHANNELS> channelOrder;
    std::array<std::array<bool, CHANNELS>, 2> gates;
    std::array<bool, CHANNELS> lastGateInputStatus;
    std::array<bool, CHANNELS> updateOutputChannelValues;
    std::deque<size_t> recentChannels;
    size_t polyMode = MONO_MODE;
    size_t lastRotationChannel = 0;
    bool delayEnabled = false;
    bool lastDelayStatus = false;
    

    Psychopump() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        std::string label;
        for(size_t i = 0; i < CHANNELS; i++) {
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

            configParam(CV0_PLUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Add random offset");
            configParam(CV1_PLUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Add random offset");
            configParam(CV2_PLUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Add random offset");
            configParam(CV3_PLUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Add random offset");
            configParam(CV4_PLUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Add random offset");
            configParam(CV5_PLUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Add random offset");
            configParam(CV6_PLUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Add random offset");
            configParam(CV7_PLUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Add random offset");
            configParam(SH0_PLUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Add random offset");
            configParam(SH1_PLUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Add random offset");
            configParam(PITCH_PLUS_RANDOM_PARAM + i, 0.f, 1.f, 0.f, "Add random offset");

            configParam(CV0_MINUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Subtract random offset");
            configParam(CV1_MINUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Subtract random offset");
            configParam(CV2_MINUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Subtract random offset");
            configParam(CV3_MINUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Subtract random offset");
            configParam(CV4_MINUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Subtract random offset");
            configParam(CV5_MINUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Subtract random offset");
            configParam(CV6_MINUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Subtract random offset");
            configParam(CV7_MINUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Subtract random offset");
            configParam(SH0_MINUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Subtract random offset");
            configParam(SH1_MINUS_RANDOM_PARAM   + i, 0.f, 1.f, 0.f, "Subtract random offset");
            configParam(PITCH_MINUS_RANDOM_PARAM + i, 0.f, 1.f, 0.f, "Subtract random offset");
            
            configParam(GATE_LENGTH_PARAM + i, 0.f, 10.f, 0.f, "Change Gate length");

            configParam(MUTE_PARAM + i, 0.f, 1.f, 0.f, "Mute");
            configParam(SOLO_PARAM + i, 0.f, 1.f, 0.f, "Solo");
            configParam(OUT0_PARAM + i, 0.f, 1.f, 1.f, "Gate to Output 1");
            configParam(OUT1_PARAM + i, 0.f, 1.f, 0.f, "Gate to Output 2");

            label = "Randomize knobs on Channel ";
            label.append(std::to_string(i + 1));
            configParam(RANDOMIZE_PARAM + i, 0.f, 1.f, 0.f, label);

            configParam(QUANTIZE_PARAM + i, 0.f, 1.f, 0.f, "Quantize");

            configParam(CHANNEL_LABEL_PARAM + i, 0.f, 1.f, 0.f, "Add Channel label on LCD");
            configParam(OUTPUT_LABEL_PARAM + i, 0.f, 1.f, 0.f, "Add Output label on LCD");

            label = "Random offset for Knob ";
            label.append(std::to_string(i + 1));
            configParam(CV_RANDOM_OFFSET_PARAM + i, 0.f, 5.f, 0.f, label, "V");
            configParam(CV_POLARITY_PARAM + i, 0.f, 1.f, 0.f, "Unipolar / Bipolar");
        }
        configParam(SH0_RANDOM_OFFSET_PARAM, 0.f, 5.f, 0.f, "Random offset for Sample & Hold 1");
        configParam(SH1_RANDOM_OFFSET_PARAM, 0.f, 5.f, 0.f, "Random offset for Sample & Hold 2");
        configParam(PITCH_RANDOM_OFFSET_PARAM, 0.f, 12.f, 0.f, "Random offset for Pitch", " semitones");
        configParam(POLY_MODE_PARAM, 0.f, 1.f, 0.f, "Polyphony Mode");
        configParam(GATE_DELAY_PARAM, 0.f, 1.f, 0.f, "5ms gate delay for modules that snapshot CV on trigger");

        onReset();
    }


    void onReset() override {
        std::string label;

        lcdStatus.text1 = "Insert Obol";
        lcdStatus.text2 = " To Depart";
        lcdStatus.layout = Lcd::TEXT1_AND_TEXT2_LAYOUT;
        lcdStatus.lastInteraction = 0.f;
        lcdStatus.dirty = true;
        recentChannels.clear();
        polyMode = MONO_MODE;
        lastRotationChannel = 0;
        delayEnabled = false;
        lastDelayStatus = false;

        for(size_t i = 0; i < CHANNELS; i++) {
            channelOrder[i] = i;
            gates[0][i] = false;
            gates[1][i] = false;
            lastGateInputStatus[i] = false;
            updateOutputChannelValues[i] = false;
            outputChannels[i].reset();

            label = "Gate ";
            label.append(std::to_string(i + 1));
            channelLabels[i] = label;

            label = "Knob ";
            label.append(std::to_string(i + 1));
            outputLabels[i] = label;
        }
    }


    json_t* dataToJson() override {
        json_t *rootJ = json_object();

        json_t *channelLabelsJ = json_array();
        for (size_t i = 0; i < CHANNELS; i++) {
            json_array_insert_new(channelLabelsJ, i, json_string(channelLabels[i].c_str()) );
        }
        json_object_set_new(rootJ, "channelLabels", channelLabelsJ);

        json_t *outputLabelsJ = json_array();
        for (size_t i = 0; i < CHANNELS; i++) {
            json_array_insert_new(outputLabelsJ, i, json_string(outputLabels[i].c_str()) );
        }
        json_object_set_new(rootJ, "outputLabels", outputLabelsJ);

        json_object_set_new(rootJ, "delayEnabled", json_boolean(delayEnabled));
        json_object_set_new(rootJ, "polyMode", json_integer(polyMode));

        return rootJ;
    }


    void dataFromJson(json_t* rootJ) override {
        json_t *channelLabelsJ = json_object_get(rootJ, "channelLabels");
        if (channelLabelsJ) {
            for (size_t i = 0; i < CHANNELS; i++) {
                json_t *channelLabelJ = json_array_get(channelLabelsJ, i);
                if (channelLabelJ) {
                    channelLabels[i] = json_string_value(channelLabelJ);
                }
            }
        }

        json_t *outputLabelsJ = json_object_get(rootJ, "outputLabels");
        if (outputLabelsJ) {
            for (size_t i = 0; i < CHANNELS; i++) {
                json_t *outputLabelJ = json_array_get(outputLabelsJ, i);
                if (outputLabelJ) {
                    outputLabels[i] = json_string_value(outputLabelJ);
                }
            }
        }

        json_t* delayEnabledJ = json_object_get(rootJ, "delayEnabled");
        if (delayEnabledJ) delayEnabled = json_boolean_value(delayEnabledJ);

        json_t* polyModeJ = json_object_get(rootJ, "polyMode");
        if (polyModeJ) polyMode = json_integer_value(polyModeJ);
    }


    // Randomizing the whole device instead of channels already makes no sense, but 
    // we definitely never want to randomize the Mute/Solo status.
    void onRandomize() override {
        for(size_t i = 0; i < CHANNELS; i++) {
            params[MUTE_PARAM + i].setValue(0.f);
            params[SOLO_PARAM + i].setValue(0.f);
        }
    }


    struct RandomizeAction : history::ModuleAction {
        std::array<float, 8> oldValues;
        std::array<float, 8> newValues;
        size_t channel;

        RandomizeAction(int _moduleId, std::string _name, size_t _channel, std::array<float, 8> _oldValues, std::array<float, 8> _newValues) {
            moduleId = _moduleId;
            name = _name;
            channel = _channel;
            oldValues = _oldValues;
            newValues = _newValues;
        }

        void undo() override {
            Psychopump *module = dynamic_cast<Psychopump*>(APP->engine->getModule(this->moduleId));
            assert(module);
            module->params[CV0_PARAM + channel].setValue(oldValues[0]);
            module->params[CV1_PARAM + channel].setValue(oldValues[1]);
            module->params[CV2_PARAM + channel].setValue(oldValues[2]);
            module->params[CV3_PARAM + channel].setValue(oldValues[3]);
            module->params[CV4_PARAM + channel].setValue(oldValues[4]);
            module->params[CV5_PARAM + channel].setValue(oldValues[5]);
            module->params[CV6_PARAM + channel].setValue(oldValues[6]);
            module->params[CV7_PARAM + channel].setValue(oldValues[7]);
        }

        void redo() override {
            Psychopump *module = dynamic_cast<Psychopump*>(APP->engine->getModule(this->moduleId));
            assert(module);
            module->params[CV0_PARAM + channel].setValue(this->newValues[0]);
            module->params[CV1_PARAM + channel].setValue(this->newValues[1]);
            module->params[CV2_PARAM + channel].setValue(this->newValues[2]);
            module->params[CV3_PARAM + channel].setValue(this->newValues[3]);
            module->params[CV4_PARAM + channel].setValue(this->newValues[4]);
            module->params[CV5_PARAM + channel].setValue(this->newValues[5]);
            module->params[CV6_PARAM + channel].setValue(this->newValues[6]);
            module->params[CV7_PARAM + channel].setValue(this->newValues[7]);
        }
    };


    void randomizeCv(size_t channel){
        std::array<float, 8> oldValues;
        std::array<float, 8> newValues;

        oldValues[0] = params[CV0_PARAM + channel].getValue();
        oldValues[1] = params[CV1_PARAM + channel].getValue();
        oldValues[2] = params[CV2_PARAM + channel].getValue();
        oldValues[3] = params[CV3_PARAM + channel].getValue();
        oldValues[4] = params[CV4_PARAM + channel].getValue();
        oldValues[5] = params[CV5_PARAM + channel].getValue();
        oldValues[6] = params[CV6_PARAM + channel].getValue();
        oldValues[7] = params[CV7_PARAM + channel].getValue();

        params[CV0_PARAM + channel].setValue(random::uniform() * 10.f);
        params[CV1_PARAM + channel].setValue(random::uniform() * 10.f);
        params[CV2_PARAM + channel].setValue(random::uniform() * 10.f);
        params[CV3_PARAM + channel].setValue(random::uniform() * 10.f);
        params[CV4_PARAM + channel].setValue(random::uniform() * 10.f);
        params[CV5_PARAM + channel].setValue(random::uniform() * 10.f);
        params[CV6_PARAM + channel].setValue(random::uniform() * 10.f);
        params[CV7_PARAM + channel].setValue(random::uniform() * 10.f);

        newValues[0] = params[CV0_PARAM + channel].getValue();
        newValues[1] = params[CV1_PARAM + channel].getValue();
        newValues[2] = params[CV2_PARAM + channel].getValue();
        newValues[3] = params[CV3_PARAM + channel].getValue();
        newValues[4] = params[CV4_PARAM + channel].getValue();
        newValues[5] = params[CV5_PARAM + channel].getValue();
        newValues[6] = params[CV6_PARAM + channel].getValue();
        newValues[7] = params[CV7_PARAM + channel].getValue();

        APP->history->push(new RandomizeAction(this->id, "randomize Psychopump channel CV", channel, oldValues, newValues));
    }


    void updateRandomizeButtons() {
        for (size_t i = 0; i < CHANNELS; i++) {
            if (randomizeCvTrigger[i].process(params[RANDOMIZE_PARAM + i].getValue())) randomizeCv(i);
        }
    }


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


    // Empties every channel's delay queue & sustaining gate if the delay was just enabled or disabled
    void updateDelayStatus() {
        delayEnabled = (params[GATE_DELAY_PARAM].getValue() == 1.f) ? true : false;
        if (delayEnabled != lastDelayStatus) {
            for(size_t i = 0; i < CHANNELS; i++) outputChannels[i].emptyQueue();
        }
        lastDelayStatus = delayEnabled;
    }
   

    // True if not muted or excluded by a solo
    bool channelEnabled(size_t channel) {
        // Is there a solo?
        bool soloEnabled = false;
        for (size_t i = 0; i < CHANNELS; i++) {
            if (params[SOLO_PARAM + i].getValue() == 1.f) soloEnabled = true;
        }
        if (soloEnabled) {
            if (params[SOLO_PARAM + channel].getValue() == 1.f && params[MUTE_PARAM + channel].getValue() == 0.f) {
                return true;
            } else {
                return false;
            }
        }
        // There's no solo, so is there a mute?
        if (params[MUTE_PARAM + channel].getValue() == 0.f) {
            return true;
        } else {
            return false;
        }
    }


    void updateGates() {
        for (size_t i = 0; i < CHANNELS; i++) {
            bool gateStatus = (inputs[GATE_INPUT + i].getVoltageSum() > 0.1f && channelEnabled(i));
            float gateDuration = params[GATE_LENGTH_PARAM + i].getValue();

            if (gateDuration > 0.00001f ) {
                gateDuration = rescale(gateDuration, 0.f, 10.f, -3.0f, 1.0f);
                gateDuration = powf(10.0f, gateDuration);
                gateDuration = gateDuration * 1000;
            } else {
                gateDuration = 0.f;
            }

            if (gateDuration > 0.f) {
                // When the gate length is set, interpret input as a trigger
                if (lastGateInputStatus[i]) {
                    outputChannels[i].updateGate(false, 0, delayEnabled, 0);
                    outputChannels[i].updateGate(false, 0, delayEnabled, 1);
                } else {
                    outputChannels[i].updateGate((gateStatus && params[OUT0_PARAM + i].getValue() == 1.f), (size_t) gateDuration, delayEnabled, 0);
                    outputChannels[i].updateGate((gateStatus && params[OUT1_PARAM + i].getValue() == 1.f), (size_t) gateDuration, delayEnabled, 1);
                    updateOutputChannelValues[i] = gateStatus;
                    if (gateStatus) recentChannels.push_front(i);
                }
            } else {
                // When the gate length is 0, pass input as-is as a gate, but update values only once.
                outputChannels[i].updateGate((gateStatus && params[OUT0_PARAM + i].getValue() == 1.f), 0, delayEnabled, 0);
                outputChannels[i].updateGate((gateStatus && params[OUT1_PARAM + i].getValue() == 1.f), 0, delayEnabled, 1);
                if (!lastGateInputStatus[i]) {
                    updateOutputChannelValues[i] = gateStatus;
                    if (gateStatus) recentChannels.push_front(i);
                }
            }

            lastGateInputStatus[i] = gateStatus;

            gates[0][i] = outputChannels[i].getGateVoltage(0);
            gates[1][i] = outputChannels[i].getGateVoltage(1);
        }
    }


    float applyVoltageOffset(const float& value, const float& offset, const float& minus, const float& plus, const float& polarity = 0.f) {
        bool minusMode = (minus == 1.f);
        bool plusMode = (plus == 1.f);
        if (minusMode && plusMode) {
            if (random::uniform() > 0.5f) {
                minusMode = false;
            } else {
                plusMode = false;
            }
        }
        float v = value;
        if (polarity == 1.f) v -= 5.f;
        if (minusMode) v -= offset * random::uniform();
        if (plusMode)  v += offset * random::uniform();
        return v;
    }


    // If nothing in the input, tries the ones above until it finds one, or uses the first if nothing found.
    float getForwardedVoltage(size_t input, size_t channel) {
        for (size_t i = channel; i > 0; i--) {
            if (inputs[input + i].isConnected()) return inputs[input + i].getVoltage(0);
        }
        return inputs[input].getVoltage(0);
    }


    // Called by updateValues() when necessary, as part of process(), and when turning knobs manually.
    void setOutputValues(size_t channel, bool randomOffsets) {
        outputChannels[channel].cv[0] = applyVoltageOffset(params[CV0_PARAM + channel].getValue(), params[CV_RANDOM_OFFSET_PARAM + 0].getValue(), params[CV0_MINUS_RANDOM_PARAM].getValue(), params[CV0_PLUS_RANDOM_PARAM].getValue(), params[CV_POLARITY_PARAM + 0].getValue());
        outputChannels[channel].cv[1] = applyVoltageOffset(params[CV1_PARAM + channel].getValue(), params[CV_RANDOM_OFFSET_PARAM + 1].getValue(), params[CV1_MINUS_RANDOM_PARAM].getValue(), params[CV1_PLUS_RANDOM_PARAM].getValue(), params[CV_POLARITY_PARAM + 1].getValue());
        outputChannels[channel].cv[2] = applyVoltageOffset(params[CV2_PARAM + channel].getValue(), params[CV_RANDOM_OFFSET_PARAM + 2].getValue(), params[CV2_MINUS_RANDOM_PARAM].getValue(), params[CV2_PLUS_RANDOM_PARAM].getValue(), params[CV_POLARITY_PARAM + 2].getValue());
        outputChannels[channel].cv[3] = applyVoltageOffset(params[CV3_PARAM + channel].getValue(), params[CV_RANDOM_OFFSET_PARAM + 3].getValue(), params[CV3_MINUS_RANDOM_PARAM].getValue(), params[CV3_PLUS_RANDOM_PARAM].getValue(), params[CV_POLARITY_PARAM + 3].getValue());
        outputChannels[channel].cv[4] = applyVoltageOffset(params[CV4_PARAM + channel].getValue(), params[CV_RANDOM_OFFSET_PARAM + 4].getValue(), params[CV4_MINUS_RANDOM_PARAM].getValue(), params[CV4_PLUS_RANDOM_PARAM].getValue(), params[CV_POLARITY_PARAM + 4].getValue());
        outputChannels[channel].cv[5] = applyVoltageOffset(params[CV5_PARAM + channel].getValue(), params[CV_RANDOM_OFFSET_PARAM + 5].getValue(), params[CV5_MINUS_RANDOM_PARAM].getValue(), params[CV5_PLUS_RANDOM_PARAM].getValue(), params[CV_POLARITY_PARAM + 5].getValue());
        outputChannels[channel].cv[6] = applyVoltageOffset(params[CV6_PARAM + channel].getValue(), params[CV_RANDOM_OFFSET_PARAM + 6].getValue(), params[CV6_MINUS_RANDOM_PARAM].getValue(), params[CV6_PLUS_RANDOM_PARAM].getValue(), params[CV_POLARITY_PARAM + 6].getValue());
        outputChannels[channel].cv[7] = applyVoltageOffset(params[CV7_PARAM + channel].getValue(), params[CV_RANDOM_OFFSET_PARAM + 7].getValue(), params[CV7_MINUS_RANDOM_PARAM].getValue(), params[CV7_PLUS_RANDOM_PARAM].getValue(), params[CV_POLARITY_PARAM + 7].getValue());
        outputChannels[channel].sh[0] = applyVoltageOffset(getForwardedVoltage(SH0_INPUT, channel), params[SH0_RANDOM_OFFSET_PARAM].getValue(), params[SH0_MINUS_RANDOM_PARAM].getValue(), params[SH0_PLUS_RANDOM_PARAM].getValue());
        outputChannels[channel].sh[1] = applyVoltageOffset(getForwardedVoltage(SH1_INPUT, channel), params[SH1_RANDOM_OFFSET_PARAM].getValue(), params[SH1_MINUS_RANDOM_PARAM].getValue(), params[SH1_PLUS_RANDOM_PARAM].getValue());

        // Pitch. That one's a pain.
        float pitch = getForwardedVoltage(PITCH_INPUT, channel);
        // Calculate offset
        bool minusMode = (params[PITCH_MINUS_RANDOM_PARAM].getValue() == 1.f);
        bool plusMode = (params[PITCH_PLUS_RANDOM_PARAM].getValue() == 1.f);
        if (minusMode && plusMode) {
            if (random::uniform() > 0.5f) {
                minusMode = false;
            } else {
                plusMode = false;
            }
        }
        int offset = 0;
        if (plusMode)  offset =  random::u32() % ( (int) params[PITCH_RANDOM_OFFSET_PARAM].getValue() + 1 );
        if (minusMode) offset = (random::u32() % ( (int) params[PITCH_RANDOM_OFFSET_PARAM].getValue() + 1 )) * -1;
        // Calculate pitch depending on mode
        if (params[QUANTIZE_PARAM + channel].getValue() == 0.f) {
            // No quantization
            pitch = pitch + ((float) offset * (1.f / 12.f));
        } else {
            // What scale to use
            std::array<bool, 12> scale;
            if (inputs[POLY_EXTERNAL_SCALE_INPUT].isConnected()) {
                for (size_t i = 0; i < 12; i++) {
                    scale[i] = (inputs[POLY_EXTERNAL_SCALE_INPUT].getVoltage(i) > 0.1);
                }
            } else {
                scale = Quantizer::validNotesInScale(Quantizer::CHROMATIC);
            }
            // Quantize
            pitch = pitch + ((float) offset * (1.f / 12.f));
            pitch = Quantizer::quantize(pitch, scale);
        }
        outputChannels[channel].pitch = pitch;

        // Keep them all to -10.f ~ 10.f
        outputChannels[channel].clampValues();
    }

    void updateValues() {
        for (size_t i = 0; i < CHANNELS; i++) {
            if (updateOutputChannelValues[i]) setOutputValues(i, true);
            updateOutputChannelValues[i] = false;
        }
    }

    bool isInFirst3Channels(size_t channel) {
        if ( channelOrder[0] == channel ) return true;
        if ( channelOrder[1] == channel ) return true;
        if ( channelOrder[2] == channel ) return true;
        return false;
    }

    void pushToRotation(size_t channel) {
        if (lastRotationChannel == 2) {
            channelOrder[0] = channel;
            lastRotationChannel = 0;
        } else {
            channelOrder[lastRotationChannel + 1] = channel;
            lastRotationChannel++;
        }
    }

    void updateChannels() {
        if (polyMode == MONO_MODE) { 
            if (!recentChannels.empty()) channelOrder[0] = recentChannels.front();
        }
        if (polyMode == ROTATE_MODE) {
            for (size_t i = 0; i < 3; i++) {
                if (!recentChannels.empty()) {
                    if (! isInFirst3Channels(recentChannels.front()) ) {
                        pushToRotation(recentChannels.front());
                    }
                    recentChannels.pop_front();
                }
            }
        }
        if (polyMode == ALL_MODE) { 
            for(size_t i = 0; i < CHANNELS; i++) channelOrder[i] = i;
        }
        recentChannels.clear();
    }


    void sendOutput() {
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
            outputs[SH0_OUTPUT].setVoltage(outputChannels[ channelOrder[i] ].sh[0], i);
            outputs[SH1_OUTPUT].setVoltage(outputChannels[ channelOrder[i] ].sh[1], i);
            outputs[PITCH_OUTPUT].setVoltage(outputChannels[ channelOrder[i] ].pitch, i);
            outputs[GATE0_OUTPUT].setVoltage((gates[0][ channelOrder[i] ] ? 10.f : 0.f), i);
            outputs[GATE1_OUTPUT].setVoltage((gates[1][ channelOrder[i] ] ? 10.f : 0.f), i);
        }
        outputs[CV0_OUTPUT].setChannels(max);
        outputs[CV1_OUTPUT].setChannels(max);
        outputs[CV2_OUTPUT].setChannels(max);
        outputs[CV3_OUTPUT].setChannels(max);
        outputs[CV4_OUTPUT].setChannels(max);
        outputs[CV5_OUTPUT].setChannels(max);
        outputs[CV6_OUTPUT].setChannels(max);
        outputs[CV7_OUTPUT].setChannels(max);
        outputs[SH0_OUTPUT].setChannels(max);
        outputs[SH1_OUTPUT].setChannels(max);
        outputs[PITCH_OUTPUT].setChannels(max);
        outputs[GATE0_OUTPUT].setChannels(max);
        outputs[GATE1_OUTPUT].setChannels(max);
    }


    void updateLights() {
        // Decaying lights are 4x slower than the suggested rate
        if (polyMode == MONO_MODE) {
            for (size_t i = 0; i < CHANNELS; i++) {
                lights[CHANNEL_LIGHT + i].setSmoothBrightness(( channelOrder[0] == i ) ? 1.f : 0.f, 0.00025f);
            }
        }
        if (polyMode == ROTATE_MODE) {
            for (size_t i = 0; i < CHANNELS; i++) {
                lights[CHANNEL_LIGHT + i].setSmoothBrightness(( channelOrder[0] == i ) ? 1.f : 0.f, 0.00025f);
                lights[CHANNEL_LIGHT + i].setSmoothBrightness(( channelOrder[1] == i ) ? 1.f : 0.f, 0.00025f);
                lights[CHANNEL_LIGHT + i].setSmoothBrightness(( channelOrder[2] == i ) ? 1.f : 0.f, 0.00025f);
            }
        }
        if (polyMode == ALL_MODE) {
            for (size_t i = 0; i < CHANNELS; i++) {
                lights[CHANNEL_LIGHT + i].setSmoothBrightness(( i < getMaxPolyphonyChannels() ) ? 1.f : 0.f, 0.00025f);
            }
        }
        lights[GATE0_LIGHT].setSmoothBrightness( (outputs[GATE0_OUTPUT].getVoltage(0) == 10.f) ? 1.f : 0.f, 0.00025f);
        lights[GATE1_LIGHT].setSmoothBrightness( (outputs[GATE1_OUTPUT].getVoltage(0) == 10.f) ? 1.f : 0.f, 0.00025f);
        lights[POLY_MODE_MONO_LIGHT].setBrightness((polyMode == MONO_MODE) ? 1.f : 0.f);
        lights[POLY_MODE_ROTATE_LIGHT].setBrightness((polyMode == ROTATE_MODE) ? 1.f : 0.f);
        lights[POLY_MODE_ALL_LIGHT].setBrightness((polyMode == ALL_MODE) ? 1.f : 0.f);
    }
    

    void process(const ProcessArgs& args) override {

        lcdStatus.processLcd(args.sampleTime);

        // We want the module to run at 1hz or below, regardless of sample rate, so that gates and
        // silences are always at least 1ms.
        // https://community.vcvrack.com/t/what-is-the-shortest-safe-gap-in-between-gates-to-retrigger/11303
        if (processTimer.process(args.sampleTime) > 0.001f) {
            processTimer.reset();
            updateRandomizeButtons();
            updatePolyphonyMode();
            updateDelayStatus();
            updateGates();
            updateValues();
            updateChannels();
            sendOutput();
            updateLights();
        }
    }
};




// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------



struct PsychopumpLcdWidget : TransparentWidget {
    Psychopump *module;
    Lcd::LcdFramebufferWidget<Psychopump> *lfb;
    Lcd::LcdDrawWidget<Psychopump> *ldw;
    std::array<bool, 8> gate0Status;
    std::array<bool, 8> gate1Status;
    std::array<bool, 8> lastGate0Status;
    std::array<bool, 8> lastGate1Status;
    std::string s;
    bool timeoutDirty = false;

    PsychopumpLcdWidget(Psychopump *_module){
        module = _module;
        lfb = new Lcd::LcdFramebufferWidget<Psychopump>(module); 
        ldw = new Lcd::LcdDrawWidget<Psychopump>(module, "Insert Obol", " To Depart");
        addChild(lfb);
        lfb->addChild(ldw);
        for (size_t i = 0; i < 8; i++) gate0Status[i] = false;
        for (size_t i = 0; i < 8; i++) gate1Status[i] = false;
        for (size_t i = 0; i < 8; i++) lastGate0Status[i] = false;
        for (size_t i = 0; i < 8; i++) lastGate1Status[i] = false;
        s = "";
    }

    void processDefaultMode() {
        if (!module) return;
        if (module->lcdStatus.lastInteraction != -1.f) {
            timeoutDirty = true;
            return;
        }
        for (size_t i = 0; i < 8; i++) gate0Status[i] = module->outputChannels[i].gateStatus[0] || module->outputChannels[i].gateDisplay[0];
        for (size_t i = 0; i < 8; i++) gate1Status[i] = module->outputChannels[i].gateStatus[1] || module->outputChannels[i].gateDisplay[1];
        if (gate0Status != lastGate0Status || gate1Status != lastGate1Status || timeoutDirty) {
            s = "1:";
            for (size_t i = 0; i < 8; i++) s.append( ( gate0Status[i] && module->params[module->OUT0_PARAM + i].getValue() == 1.f) ? "=" : "_" );
            module->lcdStatus.text1 = s;
            s = "2:";
            for (size_t i = 0; i < 8; i++) s.append( (gate1Status[i] && module->params[module->OUT1_PARAM + i].getValue() == 1.f) ? "=" : "_" );
            module->lcdStatus.text2 = s;
            module->lcdStatus.dirty = true;
        }
        lastGate0Status = gate0Status;
        lastGate1Status = gate1Status;
        for (size_t i = 0; i < 8; i++) {
            module->outputChannels[i].gateDisplay[0] = false;
            module->outputChannels[i].gateDisplay[1] = false;
        }
        timeoutDirty = false;
    }

    void draw(const DrawArgs& args) override {
        processDefaultMode();
        TransparentWidget::draw(args);
    }
};


struct CvKnob : W::KnobTransparent {
    Psychopump* module;
    size_t channel = 0;
    size_t output = 0;

    void onDragMove(const event::DragMove& e) override {
        module->lcdStatus.lastInteraction = 0.f;
        module->lcdStatus.dirty = true;
        module->lcdStatus.text1 = module->channelLabels[channel];
        module->lcdStatus.text2 = module->outputLabels[output];
        W::Knob::onDragMove(e);
    }
};


struct GateLengthKnob : W::Knob {
    Psychopump* module;
    size_t channel = 0;

    void onDragMove(const event::DragMove& e) override {
        module->lcdStatus.lastInteraction = 0.f;
        module->lcdStatus.dirty = true;
        module->lcdStatus.text1 = "Gate Length";
        float duration = module->params[module->GATE_LENGTH_PARAM + channel].getValue();
        if (duration > 0.01f) {
            module->lcdStatus.text2 = "-----------";
            if (duration < 1.f) {
                int displayDurationMs = duration * 1000;
                displayDurationMs = truncf(displayDurationMs);
                module->lcdStatus.text2 = std::to_string(displayDurationMs);
                module->lcdStatus.text2.append("ms");
            } else {
                module->lcdStatus.text2 = std::to_string(duration);
                module->lcdStatus.text2.resize(4);
                module->lcdStatus.text2.append("s");
            }
        } else {
            module->lcdStatus.text2 = "No Change";
        }
        W::Knob::onDragMove(e);
    }
};


// Thank you https://github.com/stoermelder/vcvrack-packone/blob/v1/src/Glue.cpp 
// for figuring out how to autofocus a field properly lol 
struct ChannelLabelField : TextField {
    Psychopump* module;
    size_t channel = 0;

    // Force close on Enter
    void onSelectKey(const event::SelectKey& e) override {
        if (e.action == GLFW_PRESS && e.key == GLFW_KEY_ENTER) {
            module->channelLabels[channel] = text;
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
        module->channelLabels[channel] = text;
        TextField::step();
    }
};


struct OutputLabelField : TextField {
    Psychopump* module;
    size_t output = 0;

    void onSelectKey(const event::SelectKey& e) override {
        if (e.action == GLFW_PRESS && e.key == GLFW_KEY_ENTER) {
            module->outputLabels[output] = text;
            getAncestorOfType<ui::MenuOverlay>()->requestDelete();
            e.consume(this);
        }
        if (!e.getTarget()) {
            TextField::onSelectKey(e);
        }
    }

    void step() override {
        APP->event->setSelected(this);
        module->outputLabels[output] = text;
        TextField::step();
    }
};


struct GateLabelButton : W::LitSvgSwitchUnshadowed {
    Psychopump* module;
    size_t channel = 0;
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
        channelLabelField->channel = channel;
        channelLabelField->box.size.x = 80.f;
        channelLabelField->placeholder = "Label";
        channelLabelField->setText(module->channelLabels[channel]);
        channelLabelField->selectAll();
        menu->addChild(channelLabelField);
        e.consume(this);
    }
};


struct OutputLabelButton : W::LitSvgSwitchUnshadowed {
    Psychopump* module;
    size_t output = 0;
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
        outputLabelField->output = output;
        outputLabelField->box.size.x = 80.f;
        outputLabelField->placeholder = "Label";
        outputLabelField->setText(module->outputLabels[output]);
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


struct Out0Button : W::LitSvgSwitchUnshadowed {
    Out0Button() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/psychopump-out1-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/psychopump-out1-on.svg")));
    }
};


struct Out1Button : W::LitSvgSwitchUnshadowed {
    Out1Button() {
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
            gateLabelButton->channel = i;
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
            addParam(createParam<Out0Button>(mm2px(Vec(xOffset + 5.2f, yOffset +  0.f + i * 10.f)), module, Psychopump::OUT0_PARAM + i));
            addParam(createParam<Out1Button>(mm2px(Vec(xOffset + 5.2f, yOffset + 4.1f + i * 10.f)), module, Psychopump::OUT1_PARAM + i));
        }
    }

    void addGateLengthControls(float xOffset, float yOffset, Psychopump* module) {
        for(size_t i = 0; i < 8; i++) {
            GateLengthKnob* gateLengthKnob = new GateLengthKnob;
            gateLengthKnob->module = module;
            gateLengthKnob->box.pos = mm2px(Vec(xOffset, yOffset + i * 10.f));
            gateLengthKnob->channel = i;
            if (module) gateLengthKnob->paramQuantity = module->paramQuantities[Psychopump::GATE_LENGTH_PARAM + i];
            addParam(gateLengthKnob);
        }
    }

    void addShInputs(float xOffset, float yOffset, Psychopump* module) {
        for(size_t i = 0; i < 8; i++) {
            addParam(createParam<PlusButton>(mm2px(Vec(xOffset + 4.1f, i * 10.f + yOffset)), module, Psychopump::SH0_PLUS_RANDOM_PARAM + i));
            addParam(createParam<MinusButton>(mm2px(Vec(xOffset + 4.1f, i * 10.f + yOffset + 3.95f)), module, Psychopump::SH0_MINUS_RANDOM_PARAM + i));
            addParam(createParam<PlusButton>(mm2px(Vec(xOffset + 4.1f + 14.f, i * 10.f + yOffset)), module, Psychopump::SH1_PLUS_RANDOM_PARAM + i));
            addParam(createParam<MinusButton>(mm2px(Vec(xOffset + 4.1f + 14.f, i * 10.f + yOffset + 3.95f)), module, Psychopump::SH1_MINUS_RANDOM_PARAM + i));
            addStaticInput(mm2px(Vec( 0.f + xOffset,  i * 10.f + yOffset)), module, Psychopump::SH0_INPUT + i);
            addStaticInput(mm2px(Vec(14.f + xOffset,  i * 10.f + yOffset)), module, Psychopump::SH1_INPUT + i);
        }
    }


    void addShRandomOffsetKnobs(float xOffset, float yOffset, Psychopump* module) {
        addParam(createParam<W::Knob>(mm2px(Vec(xOffset,        yOffset)), module, Psychopump::SH0_RANDOM_OFFSET_PARAM));
        addParam(createParam<W::Knob>(mm2px(Vec(xOffset + 14.f, yOffset)), module, Psychopump::SH1_RANDOM_OFFSET_PARAM));
    }


    void addShOutputs(float xOffset, float yOffset, Psychopump* module) {
        addStaticOutput(mm2px(Vec(xOffset, yOffset)), module, Psychopump::SH0_OUTPUT);
        addStaticOutput(mm2px(Vec(xOffset + 14.f, yOffset)), module, Psychopump::SH1_OUTPUT);
    }


    void addShSection(float xOffset, float yOffset, Psychopump* module) {
        addShInputs(xOffset, yOffset, module);
        addShRandomOffsetKnobs(xOffset + 2.9f, yOffset + 80.f, module);
        addShOutputs(xOffset + 2.9f, yOffset + 90.f, module);
    }


    void addRandomizeButtons(float xOffset, float yOffset, Psychopump* module) {
        for(size_t i = 0; i < 8; i++) {
            addParam(createParam<FortuneButton>(mm2px(Vec(xOffset, yOffset + 1.5f + i * 10.f)), module, Psychopump::RANDOMIZE_PARAM + i));
        }
    }


    void addCVParamElement(float xOffset, float yOffset, Psychopump* module, int light, int cvParam, int plusRandomParam, int minusRandomParam, size_t channel, size_t output) {
        addParam(createParam<PlusButton>(mm2px(Vec(xOffset + 4.1f, yOffset)), module, plusRandomParam));
        addParam(createParam<MinusButton>(mm2px(Vec(xOffset + 4.1f, yOffset + 3.95f)), module, minusRandomParam));
        addChild(W::createKnobLight<W::KnobLightYellow>(mm2px(Vec(xOffset, yOffset)), module, light, cvParam, 0.f, 10.f));
        CvKnob* cvKnob = new CvKnob;
        cvKnob->module = module;
        cvKnob->box.pos = mm2px(Vec(xOffset, yOffset));
        if (module) cvKnob->paramQuantity = module->paramQuantities[cvParam];
        cvKnob->channel = channel;
        cvKnob->output = output;
        addParam(cvKnob);
    }


    void addCVParams(float xOffset, float yOffset, Psychopump* module) {
        for(size_t i = 0; i < 8; i++) {
            addCVParamElement( 0 * 14.f + xOffset, i * 10.f + yOffset, module, Psychopump::CHANNEL_LIGHT + i, Psychopump::CV0_PARAM + i, Psychopump::CV0_PLUS_RANDOM_PARAM + i, Psychopump::CV0_MINUS_RANDOM_PARAM + i, i, 0);
            addCVParamElement( 1 * 14.f + xOffset, i * 10.f + yOffset, module, Psychopump::CHANNEL_LIGHT + i, Psychopump::CV1_PARAM + i, Psychopump::CV1_PLUS_RANDOM_PARAM + i, Psychopump::CV1_MINUS_RANDOM_PARAM + i, i, 1);
            addCVParamElement( 2 * 14.f + xOffset, i * 10.f + yOffset, module, Psychopump::CHANNEL_LIGHT + i, Psychopump::CV2_PARAM + i, Psychopump::CV2_PLUS_RANDOM_PARAM + i, Psychopump::CV2_MINUS_RANDOM_PARAM + i, i, 2);
            addCVParamElement( 3 * 14.f + xOffset, i * 10.f + yOffset, module, Psychopump::CHANNEL_LIGHT + i, Psychopump::CV3_PARAM + i, Psychopump::CV3_PLUS_RANDOM_PARAM + i, Psychopump::CV3_MINUS_RANDOM_PARAM + i, i, 3);
            addCVParamElement( 4 * 14.f + xOffset, i * 10.f + yOffset, module, Psychopump::CHANNEL_LIGHT + i, Psychopump::CV4_PARAM + i, Psychopump::CV4_PLUS_RANDOM_PARAM + i, Psychopump::CV4_MINUS_RANDOM_PARAM + i, i, 4);
            addCVParamElement( 5 * 14.f + xOffset, i * 10.f + yOffset, module, Psychopump::CHANNEL_LIGHT + i, Psychopump::CV5_PARAM + i, Psychopump::CV5_PLUS_RANDOM_PARAM + i, Psychopump::CV5_MINUS_RANDOM_PARAM + i, i, 5);
            addCVParamElement( 6 * 14.f + xOffset, i * 10.f + yOffset, module, Psychopump::CHANNEL_LIGHT + i, Psychopump::CV6_PARAM + i, Psychopump::CV6_PLUS_RANDOM_PARAM + i, Psychopump::CV6_MINUS_RANDOM_PARAM + i, i, 6);
            addCVParamElement( 7 * 14.f + xOffset, i * 10.f + yOffset, module, Psychopump::CHANNEL_LIGHT + i, Psychopump::CV7_PARAM + i, Psychopump::CV7_PLUS_RANDOM_PARAM + i, Psychopump::CV7_MINUS_RANDOM_PARAM + i, i, 7);
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
            outputLabelButton->output = i;
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


    PsychopumpWidget(Psychopump* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/faceplates/Psychopump.svg")));
        
        // Screws
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH * 14, 0)));
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH * 14, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH * 37, 0)));
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH * 37, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Signature
        addChild(createWidget<W::Signature>(mm2px(Vec(237.0f, 114.5f))));

        // Main Area
        addGateInputs(5.f, 24.f, module); // ~14mm
        addChannelControls(19.f, 24.f, module);  // ~10mm
        addGateLengthControls(31.f, 24.f, module); // 10mm
        addShSection(40.f, 24.f, module); // 28mm
        addRandomizeButtons(68.1f, 24.f, module); // ~7mm
        addCVParamsSection(75.f, 24.f, module); // 112mm
        addQuantizeButtons(187.1f, 24.f, module); // ~7mm
        addPitchSection(194.f, 24.f, module); // 14mm

        // Gate outputs
        addDynamicOutput(mm2px(Vec(13.2f, 114.f)), module, Psychopump::GATE0_OUTPUT, Psychopump::GATE0_LIGHT);
        addDynamicOutput(mm2px(Vec(27.2f, 114.f)), module, Psychopump::GATE1_OUTPUT, Psychopump::GATE1_LIGHT);

        // LCD
        PsychopumpLcdWidget *lcd = new PsychopumpLcdWidget(module);
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
