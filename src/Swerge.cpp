/*  Copyright (C) 2019-2020 Aria Salvatrice
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/


// Warning - this module was created with very little C++ experience, and features were 
// added to it later without regard for code quality. This is maintained exploratory code, not good design.


#include "plugin.hpp"

namespace Swerge {

struct Swerge : Module {
    enum ParamIds {
        SORT_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        ENUMS(MERGE_INPUT, 8),
        NUM_INPUTS
    };
    enum OutputIds {
        ENUMS(POLY_OUTPUT, 2),
        NUM_OUTPUTS
    };
    enum LightIds {
        ENUMS(POLY_LIGHT, 2),
        CHAIN_LIGHT,
        NUM_LIGHTS
    };
    
    dsp::ClockDivider ledDivider;
    bool chainMode;

    Swerge() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        ledDivider.setDivision(4096);
        configParam(SORT_PARAM, 0.f, 1.f, 0.f, "Sort voltages on both banks");
    }
    
    // Merge without sorting, faster
    void merge() {
        int lastMergeChannel = 0;

        // Don't waste time if there's no output connected
        if (!outputs[POLY_OUTPUT].isConnected() && !outputs[POLY_OUTPUT + 1].isConnected()) return;
        
        // Set first bank normally
        for (size_t i = 0; i < 4; i++) {
            if (inputs[MERGE_INPUT + i].isConnected()) {
                outputs[POLY_OUTPUT + 0].setVoltage(inputs[MERGE_INPUT + i].getVoltage(), i);
                lastMergeChannel = i+1;
            } else {
                outputs[POLY_OUTPUT + 0].setVoltage(0.f, i);
            }
        }
        outputs[POLY_OUTPUT + 0].setChannels(lastMergeChannel);
        
        if (chainMode) { // Chain first and second bank
            lastMergeChannel = 0;
            for (size_t i = 0; i < 8; i++) {
                if (inputs[MERGE_INPUT + i ].isConnected()) {
                    outputs[POLY_OUTPUT + 1].setVoltage(inputs[MERGE_INPUT + i].getVoltage(), i);
                    lastMergeChannel = i+1;
                } else {
                    outputs[POLY_OUTPUT + 1].setVoltage(0.f, i);
                }
            }
            outputs[POLY_OUTPUT + 1].setChannels(lastMergeChannel);
        } else { // Set second bank normally
            lastMergeChannel = 0;
            for (size_t i = 0; i < 4; i++) {
                if (inputs[MERGE_INPUT + i + 4].isConnected()) {
                    outputs[POLY_OUTPUT + 1].setVoltage(inputs[MERGE_INPUT + i + 4].getVoltage(), i);
                    lastMergeChannel = i+1;
                } else {
                    outputs[POLY_OUTPUT + 1].setVoltage(0.f, i);
                }
            }
            outputs[POLY_OUTPUT + 1].setChannels(lastMergeChannel);
        }
    }
    
    // Merge with sorting. Ugly CTRL-V code but it gets the job done.
    void mergeSort() {
        std::array<float, 8> mergedVoltages;
        size_t connected = 0;

        // Don't waste time if there's no output connected
        if (!outputs[POLY_OUTPUT].isConnected() && !outputs[POLY_OUTPUT + 1].isConnected()) return;

        // Fist bank normally
        connected = 0;
        for (size_t i = 0; i < 4; i++) {
            if (inputs[MERGE_INPUT + i].isConnected()) {
                mergedVoltages[i] = inputs[MERGE_INPUT + i].getVoltage();
                connected = i + 1;
            } else {
                mergedVoltages[i] = 0.f;
            }
        }
        std::sort(mergedVoltages.begin(), mergedVoltages.begin() + connected);		
        for (size_t i = 0; i < connected; i++)
            outputs[POLY_OUTPUT + 0].setVoltage(mergedVoltages[i], i);
        outputs[POLY_OUTPUT + 0].setChannels(connected);
        
        // Second bank depends on mode
        if (chainMode) { // Chain first and second
            connected = 0;
            for (size_t i = 0; i < 8; i++) {
                if (inputs[MERGE_INPUT + i].isConnected()) {
                    mergedVoltages[i] = inputs[MERGE_INPUT + i].getVoltage();
                    connected = i + 1;
                } else {
                    mergedVoltages[i] = 0.f;
                }
            }
            std::sort(mergedVoltages.begin(), mergedVoltages.begin() + connected);		
            for (size_t i = 0; i < connected; i++)
                outputs[POLY_OUTPUT + 1].setVoltage(mergedVoltages[i], i);
            outputs[POLY_OUTPUT + 1].setChannels(connected);
        } else { // No chaining, do 2nd normally
            connected = 0;
            for (size_t i = 0; i < 4; i++) {
                if (inputs[MERGE_INPUT + i + 4].isConnected()) {
                    mergedVoltages[i] = inputs[MERGE_INPUT + i + 4].getVoltage();
                    connected = i + 1;
                } else {
                    mergedVoltages[i] = 0.f;
                }
            }
            std::sort(mergedVoltages.begin(), mergedVoltages.begin() + connected);		
            for (size_t i = 0; i < connected; i++)
                outputs[POLY_OUTPUT + 1].setVoltage(mergedVoltages[i], i);
            outputs[POLY_OUTPUT + 1].setChannels(connected);
        }
    }
    
    void updateLeds() {
        lights[CHAIN_LIGHT].setBrightness( (chainMode) ? 1.f : 0.f);
        
        // Poly outputs
        lights[POLY_LIGHT + 0].setBrightness(0.f);
        lights[POLY_LIGHT + 1].setBrightness(0.f);
        for (size_t i = 0; i < 4; i++)
            if (inputs[MERGE_INPUT + i].isConnected()) {
                lights[POLY_LIGHT + 0].setBrightness(1.f);
                if (chainMode)
                    lights[POLY_LIGHT + 1].setBrightness(1.f);
            }
        for (size_t i = 4; i < 8; i++)
            if (inputs[MERGE_INPUT + i].isConnected())
                lights[POLY_LIGHT + 1].setBrightness(1.f);
    }
    
    void process(const ProcessArgs& args) override {
        chainMode = (outputs[POLY_OUTPUT + 0].isConnected()) ? false : true;
        (params[SORT_PARAM].getValue()) ? mergeSort() : merge();
        if (ledDivider.process())
            updateLeds();
    }	
};


struct SwergeWidget : ModuleWidget {
    SwergeWidget(Swerge* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/faceplates/Swerge.svg")));
        
        // Signature 
        addChild(createWidget<AriaSignature>(mm2px(Vec(1.0, 114.538))));

        // Screws
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        
        // Jacks, top to bottom.
        
        addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 20.0)), module, Swerge::MERGE_INPUT + 0));
        addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 28.0)), module, Swerge::MERGE_INPUT + 1));
        addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 36.0)), module, Swerge::MERGE_INPUT + 2));
        addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 44.0)), module, Swerge::MERGE_INPUT + 3));
        
        addChild(createLightCentered<AriaOutputLight>(mm2px(Vec(7.62, 54.0)), module, Swerge::POLY_LIGHT + 0));
        addOutput(createOutputCentered<AriaJackTransparent>(mm2px(Vec(7.62, 54.0)), module, Swerge::POLY_OUTPUT + 0));
        
        addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 67.0)), module, Swerge::MERGE_INPUT + 4));
        addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 75.0)), module, Swerge::MERGE_INPUT + 5));
        addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 83.0)), module, Swerge::MERGE_INPUT + 6));
        addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 91.0)), module, Swerge::MERGE_INPUT + 7));
        
        addChild(createLightCentered<AriaOutputLight>(mm2px(Vec(7.62, 101.0)), module, Swerge::POLY_LIGHT + 1));
        addOutput(createOutputCentered<AriaJackTransparent>(mm2px(Vec(7.62, 101.0)), module, Swerge::POLY_OUTPUT + 1));
        
        // Pushbutton
        addParam(createParam<AriaPushButton500>(mm2px(Vec(1.0, 107)), module, Swerge::SORT_PARAM));
        
        // Chain light
        addChild(createLightCentered<SmallLight<InputLight>>(mm2px(Vec(13.6, 58.0)), module, Swerge::CHAIN_LIGHT));
    }
};

} // namespace Swerge

Model* modelSwerge = createModel<Swerge::Swerge, Swerge::SwergeWidget>("Swerge");
