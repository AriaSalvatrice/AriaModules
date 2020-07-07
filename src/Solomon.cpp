/*  Copyright (C) 2019-2020 Aria Salvatrice
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

// Self-modifying sequencer. Internally, the slots are called "nodes", "step" refers to the movement.
// For now, only a 8-node version. If there is interest, other versions can be made later. 
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

template <size_t NODES>
struct Solomon : Module {
    enum ParamIds {
        KEY_PARAM,
        SCALE_PARAM,
        ENUMS(NODE_SUB_1_SD_PARAM, NODES),
        ENUMS(NODE_ADD_1_SD_PARAM, NODES),
        NUM_PARAMS
    };
    enum InputIds {
        SCALE_INPUT,
        STEP_QUEUE_INPUT,
        STEP_TELEPORT_INPUT,
        STEP_WALK_INPUT,
        STEP_BACK_INPUT,
        STEP_FORWARD_INPUT,
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
        CV_OUTPUT,
        GATE_OUTPUT,
        ENUMS(REACHED_OUTPUT, NODES),
        ENUMS(CHANCE_OUTPUT, NODES),
        ENUMS(LATCH_OUTPUT, NODES),
        ENUMS(NEXT_OUTPUT, NODES),
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    // Global
    int stepType = -1;
    float readWindow = -1.f; // -1 when closed
    dsp::SchmittTrigger stepQueueTrigger;
    dsp::SchmittTrigger stepTeleportTrigger;
    dsp::SchmittTrigger stepWalkTrigger;
    dsp::SchmittTrigger stepBackTrigger;
    dsp::SchmittTrigger stepForwardTrigger;

    // Per node
    float cv[NODES];
    std::array<bool, NODES> queue;
    dsp::SchmittTrigger queueTrigger[NODES];
    dsp::SchmittTrigger sub1SdTrigger[NODES];
    dsp::SchmittTrigger sub2SdTrigger[NODES];
    dsp::SchmittTrigger sub3SdTrigger[NODES];
    dsp::SchmittTrigger sub1OctTrigger[NODES];
    dsp::SchmittTrigger add1SdTrigger[NODES];
    dsp::SchmittTrigger add2SdTrigger[NODES];
    dsp::SchmittTrigger add3SdTrigger[NODES];
    dsp::SchmittTrigger add1OctTrigger[NODES];
    
    Solomon() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        clearQueue();
        for(size_t i = 0; i < NODES; i++) cv[i] = 1.f;
    }

    ~Solomon(){

    }

    size_t queueCount() {
        size_t count = 0;
        for(size_t i = 0; i < NODES; i++) {
            if (queue[i] == true) count++;
        }
        DEBUG("QUEUE COUNT: %d", count);
        return count;
    }

    // Opens a window if a step input is reached, and remembers what type it is.
    // If it's a queue input, something must be enqueued. 
    int readStepInputs() {
        if (stepQueueTrigger.process(inputs[STEP_QUEUE_INPUT].getVoltageSum()) && queueCount() > 0) return STEP_QUEUE;
        if (stepTeleportTrigger.process(inputs[STEP_TELEPORT_INPUT].getVoltageSum()))               return STEP_TELEPORT;
        if (stepWalkTrigger.process(inputs[STEP_WALK_INPUT].getVoltageSum()))                       return STEP_WALK;
        if (stepBackTrigger.process(inputs[STEP_BACK_INPUT].getVoltageSum()))                       return STEP_BACK;
        if (stepForwardTrigger.process(inputs[STEP_FORWARD_INPUT].getVoltageSum()))                 return STEP_FORWARD;
        return -1;
    }

    // FIXME: Cleared each step or not?
    void clearQueue() {
        for(size_t i = 0; i < NODES; i++) queue[i] = false;
    }

    void updateQueue() {
        for(size_t i = 0; i < NODES; i++) {
            if (queueTrigger[i].process(inputs[NODE_QUEUE_INPUT + i].getVoltageSum())) queue[i] = true;
        }
    }

    void processReadWindow() {
        updateQueue();
    }

    void processStep() {

    }

    void process(const ProcessArgs& args) override {
        if (readWindow < 0.f) {
            // We are not in a Read Window
            stepType = readStepInputs();
            if (stepType >= 0) readWindow = 0.f;
        }
        if (readWindow >= 0.f && readWindow < READWINDOWDURATION) {
            // We are in a Read Window
            processReadWindow();
            readWindow += args.sampleTime;
        }
        if (readWindow >= READWINDOWDURATION) {
            // A read window closed
            DEBUG(">READ WINDOW CLOSED!");
            processStep();
            readWindow = -1.f;
        }
    }
};



struct SolomonWidget : ModuleWidget {

    SolomonWidget(Solomon<8>* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/faceplates/Solomon.svg")));
        
        // Screws
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Global step inputs
        addInput(createInput<AriaJackIn>(mm2px(Vec(5.f, 15.f)), module, Solomon<8>::STEP_QUEUE_INPUT));
        addInput(createInput<AriaJackIn>(mm2px(Vec(5.f, 25.f)), module, Solomon<8>::STEP_TELEPORT_INPUT));
        addInput(createInput<AriaJackIn>(mm2px(Vec(5.f, 35.f)), module, Solomon<8>::STEP_WALK_INPUT));
        addInput(createInput<AriaJackIn>(mm2px(Vec(5.f, 45.f)), module, Solomon<8>::STEP_BACK_INPUT));
        addInput(createInput<AriaJackIn>(mm2px(Vec(5.f, 55.f)), module, Solomon<8>::STEP_FORWARD_INPUT));

        // Nodes
        float xOffset = 53.f;
        float yOffset = 20.f;
        for(size_t i = 0; i < 8; i++) {
            addInput(createInput<AriaJackIn>(   mm2px(Vec(xOffset +  5.f, yOffset +  0.f)), module, Solomon<8>::NODE_QUEUE_INPUT     + i));
            addInput(createInput<AriaJackIn>(   mm2px(Vec(xOffset +  0.f, yOffset + 10.f)), module, Solomon<8>::NODE_SUB_1_SD_INPUT  + i));
            addInput(createInput<AriaJackIn>(   mm2px(Vec(xOffset +  0.f, yOffset + 20.f)), module, Solomon<8>::NODE_SUB_2_SD_INPUT  + i));
            addInput(createInput<AriaJackIn>(   mm2px(Vec(xOffset +  0.f, yOffset + 30.f)), module, Solomon<8>::NODE_SUB_3_SD_INPUT  + i));
            addInput(createInput<AriaJackIn>(   mm2px(Vec(xOffset +  0.f, yOffset + 40.f)), module, Solomon<8>::NODE_SUB_1_OCT_INPUT + i));
            addInput(createInput<AriaJackIn>(   mm2px(Vec(xOffset + 10.f, yOffset + 10.f)), module, Solomon<8>::NODE_ADD_1_SD_INPUT  + i));
            addInput(createInput<AriaJackIn>(   mm2px(Vec(xOffset + 10.f, yOffset + 20.f)), module, Solomon<8>::NODE_ADD_2_SD_INPUT  + i));
            addInput(createInput<AriaJackIn>(   mm2px(Vec(xOffset + 10.f, yOffset + 30.f)), module, Solomon<8>::NODE_ADD_3_SD_INPUT  + i));
            addInput(createInput<AriaJackIn>(   mm2px(Vec(xOffset + 10.f, yOffset + 40.f)), module, Solomon<8>::NODE_ADD_1_OCT_INPUT + i));
            addOutput(createOutput<AriaJackOut>(mm2px(Vec(xOffset +  0.f, yOffset + 80.f)), module, Solomon<8>::REACHED_OUTPUT       + i));
            addOutput(createOutput<AriaJackOut>(mm2px(Vec(xOffset + 10.f, yOffset + 85.f)), module, Solomon<8>::CHANCE_OUTPUT        + i));
            addOutput(createOutput<AriaJackOut>(mm2px(Vec(xOffset +  0.f, yOffset + 90.f)), module, Solomon<8>::LATCH_OUTPUT         + i));
            addOutput(createOutput<AriaJackOut>(mm2px(Vec(xOffset + 10.f, yOffset + 95.f)), module, Solomon<8>::NEXT_OUTPUT          + i));
            xOffset += 25.f;
        }

    }
};

} // Namespace Solomon

Model* modelSolomon = createModel<Solomon::Solomon<8>, Solomon::SolomonWidget>("Solomon");
