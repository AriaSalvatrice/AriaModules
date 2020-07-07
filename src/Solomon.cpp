/*  Copyright (C) 2019-2020 Aria Salvatrice
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

// Self-modifying sequencer.
// For now, only a 8-step version. If there is interest, other versions will be made later. 
#include "plugin.hpp"

namespace Solomon {

const float READWINDOWDURATION = 1.f; // Seconds

enum StepTypes {
    STEP_QUEUE,
    STEP_TELEPORT,
    STEP_WALK,
    STEP_BACK,
    STEP_FORWARD
};

template <int STEPS>
struct Solomon : Module {
    enum ParamIds {
        KEY_PARAM,
        SCALE_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        SCALE_INPUT,
        STEP_QUEUE_INPUT,
        STEP_TELEPORT_INPUT,
        STEP_WALK_INPUT,
        STEP_BACK_INPUT,
        STEP_FORWARD_INPUT,
        ENUMS(ADD_1_SD_INPUT, STEPS),
        ENUMS(ADD_2_SD_INPUT, STEPS),
        ENUMS(ADD_3_SD_INPUT, STEPS),
        ENUMS(ADD_1_OCT_INPUT, STEPS),
        ENUMS(SUB_1_SD_INPUT, STEPS),
        ENUMS(SUB_2_SD_INPUT, STEPS),
        ENUMS(SUB_3_SD_INPUT, STEPS),
        ENUMS(SUB_1_OCT_INPUT, STEPS),
        ENUMS(QUEUE_INPUT, STEPS),
        NUM_INPUTS
    };
    enum OutputIds {
        CV_OUTPUT,
        GATE_OUTPUT,
        ENUMS(REACHED_OUTPUT, STEPS),
        ENUMS(CHANCE_OUTPUT, STEPS),
        ENUMS(LATCH_OUTPUT, STEPS),
        ENUMS(NEXT_OUTPUT, STEPS),
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    bool stepQueue = false;
    bool stepTeleport = false;
    bool stepWalk = false;
    bool stepBack = false;
    bool stepForward = false;
    float readWindow = -1.f; // -1 when closed
    dsp::SchmittTrigger stepQueueTrigger;
    dsp::SchmittTrigger stepTeleportTrigger;
    dsp::SchmittTrigger stepWalkTrigger;
    dsp::SchmittTrigger stepBackTrigger;
    dsp::SchmittTrigger stepForwardTrigger;
    
    Solomon() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        
    }

    ~Solomon(){

    }

    // Opens a window if a step input is reached, and remembers what type it is.
    // TODO: If it's a queue input, something must be enqueued. 
    void readStepInputs() {
        if (stepQueueTrigger.process(inputs[STEP_QUEUE_INPUT].getVoltageSum())) {
            stepQueue = true;
            readWindow = 0.f;
            return;
        }
        if (stepTeleportTrigger.process(inputs[STEP_TELEPORT_INPUT].getVoltageSum())) {
            stepTeleport = true;
            readWindow = 0.f;
            return;
        }
        if (stepWalkTrigger.process(inputs[STEP_WALK_INPUT].getVoltageSum())) {
            stepWalk = true;
            readWindow = 0.f;
            return;
        }
        if (stepBackTrigger.process(inputs[STEP_BACK_INPUT].getVoltageSum())) {
            stepBack = true;
            readWindow = 0.f;
            return;
        }
        if (stepForwardTrigger.process(inputs[STEP_FORWARD_INPUT].getVoltageSum())) {
            stepForward = true;
            readWindow = 0.f;
            return;
        }
    }

    void processReadWindow() {

    }

    void processStep() {
        stepQueue = false;
        stepTeleport = false;
        stepWalk = false;
        stepBack = false;
        stepForward = false;
    }

    void process(const ProcessArgs& args) override {
        if (readWindow < 0.f) {
            // We are not in a read window
            readStepInputs();
        }
        if (readWindow >= 0.f && readWindow < READWINDOWDURATION) {
            // We are in a Read Window
            processReadWindow();
            readWindow += args.sampleTime;
        }
        if (readWindow >= READWINDOWDURATION) {
            // A read window elapsed
            processStep();
            readWindow = -1.f;
        }
    }
};



struct SolomonWidget : ModuleWidget {

    SolomonWidget(Solomon<8>* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/faceplates/Solomon.svg")));
        
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addInput(createInput<AriaJackIn>(mm2px(Vec(5.f, 15.f)), module, Solomon<8>::STEP_QUEUE_INPUT));
        addInput(createInput<AriaJackIn>(mm2px(Vec(5.f, 25.f)), module, Solomon<8>::STEP_TELEPORT_INPUT));
        addInput(createInput<AriaJackIn>(mm2px(Vec(5.f, 35.f)), module, Solomon<8>::STEP_WALK_INPUT));
        addInput(createInput<AriaJackIn>(mm2px(Vec(5.f, 45.f)), module, Solomon<8>::STEP_BACK_INPUT));
        addInput(createInput<AriaJackIn>(mm2px(Vec(5.f, 55.f)), module, Solomon<8>::STEP_FORWARD_INPUT));

    }
};

} // Namespace Solomon

Model* modelSolomon = createModel<Solomon::Solomon<8>, Solomon::SolomonWidget>("Solomon");
