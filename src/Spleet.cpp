/*  Copyright (C) 2019-2020 Aria Salvatrice
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/


// Warning - this module was created with very little C++ experience, and features were 
// added to it later without regard for code quality. This is maintained exploratory code, not good design.


#include "plugin.hpp"

namespace Spleet {

struct Spleet : Module {
    enum ParamIds {
        SORT_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        ENUMS(POLY_INPUT, 2),
        NUM_INPUTS
    };
    enum OutputIds {
        ENUMS(SPLIT_OUTPUT, 8),
        NUM_OUTPUTS
    };
    enum LightIds {
        POLY_LIGHT,
        ENUMS(SPLIT_LIGHT, 8),
        CHAIN_LIGHT,
        NUM_LIGHTS
    };
    
    dsp::ClockDivider ledDivider;
    bool chainMode;

    Spleet() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        ledDivider.setDivision(4096);
        configParam(SORT_PARAM, 0.f, 1.f, 0.f, "Sort voltages on both banks");
        for (int i = 0; i < 2; i++)
            configInput(POLY_INPUT + i, string::f("Channel %d", i + 1));
        for (int i = 0; i < 4; i++) {
            configOutput(SPLIT_OUTPUT + i, string::f("Channel 1-%d", i + 1));
            configOutput(SPLIT_OUTPUT + 4 + i, string::f("Channel 2-%d", i + 1));
        }
    }

    // Split without sorting, faster
    void split() {
        for (size_t i = 0; i < 4; i++) // First bank
            outputs[SPLIT_OUTPUT + i].setVoltage(inputs[POLY_INPUT + 0].getVoltage(i));
        if (chainMode) { // Second bank depends on chain mode
            for (size_t i = 4; i < 8; i++) 
                outputs[SPLIT_OUTPUT + i].setVoltage(inputs[POLY_INPUT + 0].getVoltage(i));
        } else {
            for (size_t i = 4; i < 8; i++)
                outputs[SPLIT_OUTPUT + i].setVoltage(inputs[POLY_INPUT + 1].getVoltage(i - 4));
        }
    }
    

    // Split with sorting
    void splitSort() {
        std::array<float, 8> splitVoltages;	// First bank, or both if chained
        size_t connected = 0; // First bank, or both if chained
        std::array<float, 4> splitVoltagesSecond;	
        int connectedSecond = 0;
        
        // How many connected inputs?
        connected = inputs[POLY_INPUT + 0].getChannels();
        connected = ( (!chainMode) and (connected > 4)) ? 4 : connected;
        connectedSecond = inputs[POLY_INPUT + 1].getChannels();
                
        // Fill arrays
        for (size_t i = 0; i < 8; i++)
            splitVoltages[i] = (i < connected) ? (inputs[POLY_INPUT + 0].getVoltage(i)) : 0.f;
        for (int i = 0; i < 4; i++)
            splitVoltagesSecond[i] = (i < connectedSecond) ? (inputs[POLY_INPUT + 1].getVoltage(i)) : 0.f;
                    
        // Sort and output
        std::sort(splitVoltages.begin(), splitVoltages.begin() + connected);
        std::sort(splitVoltagesSecond.begin(), splitVoltagesSecond.begin() + connectedSecond);
        for (size_t i = 0; i < 4; i++)
            outputs[SPLIT_OUTPUT + i].setVoltage(splitVoltages[i]);	
        for (size_t i = 0; i < 4; i++)
            outputs[SPLIT_OUTPUT + i + 4].setVoltage( (chainMode) ? (splitVoltages[i + 4]) : (splitVoltagesSecond[i]) );
    }
    
    void updateLeds() {
        lights[CHAIN_LIGHT].setBrightness( (chainMode) ? 1.f : 0.f);
        
        // Split outputs
        for (int i = 0; i < 4; i++)
            lights[SPLIT_LIGHT + i].setBrightness( (inputs[POLY_INPUT + 0].getChannels() > i) ? 1.f : 0.f);
        if (chainMode) {
            for (int i = 4; i < 8; i++)
                lights[SPLIT_LIGHT + i].setBrightness( (inputs[POLY_INPUT + 0].getChannels() > i) ? 1.f : 0.f);
        } else {
            for (int i = 0; i < 4; i++)
                lights[SPLIT_LIGHT + i + 4].setBrightness( (inputs[POLY_INPUT + 1].getChannels() > i) ? 1.f : 0.f);
        }
    }
    
    void process(const ProcessArgs& args) override {
        chainMode = (inputs[POLY_INPUT + 1].isConnected()) ? false : true;
        (params[SORT_PARAM].getValue()) ? splitSort() : split();
        if (ledDivider.process())
            updateLeds();
    }	
};


struct SpleetWidget : W::ModuleWidget {
    SpleetWidget(Spleet* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/faceplates/Spleet.svg")));
        
        // Signature 
        addChild(createWidget<W::Signature>(mm2px(Vec(1.f, 114.5f))));

        // Screws
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        
        // Jacks, top to bottom.
        addStaticInput(mm2px(Vec(3.52f, 15.9f)), module, Spleet::POLY_INPUT + 0);       
        addDynamicOutput(mm2px(Vec(3.52f, 25.9f)), module, Spleet::SPLIT_OUTPUT + 0, Spleet::SPLIT_LIGHT + 0);
        addDynamicOutput(mm2px(Vec(3.52f, 33.9f)), module, Spleet::SPLIT_OUTPUT + 1, Spleet::SPLIT_LIGHT + 1);
        addDynamicOutput(mm2px(Vec(3.52f, 41.9f)), module, Spleet::SPLIT_OUTPUT + 2, Spleet::SPLIT_LIGHT + 2);
        addDynamicOutput(mm2px(Vec(3.52f, 49.9f)), module, Spleet::SPLIT_OUTPUT + 3, Spleet::SPLIT_LIGHT + 3);

        addStaticInput(mm2px(Vec(3.52f, 62.9f)), module, Spleet::POLY_INPUT + 1);
        addDynamicOutput(mm2px(Vec(3.52f, 72.9f)), module, Spleet::SPLIT_OUTPUT + 4, Spleet::SPLIT_LIGHT + 4);
        addDynamicOutput(mm2px(Vec(3.52f, 80.9f)), module, Spleet::SPLIT_OUTPUT + 5, Spleet::SPLIT_LIGHT + 5);
        addDynamicOutput(mm2px(Vec(3.52f, 88.9f)), module, Spleet::SPLIT_OUTPUT + 6, Spleet::SPLIT_LIGHT + 6);
        addDynamicOutput(mm2px(Vec(3.52f, 96.9f)), module, Spleet::SPLIT_OUTPUT + 7, Spleet::SPLIT_LIGHT + 7);
                
        // Pushbutton
        addParam(createParam<W::SmallButton>(mm2px(Vec(1.f, 107.f)), module, Spleet::SORT_PARAM));
        
        // Chain light
        addChild(createLight<W::StatusLightInput>(mm2px(Vec(12.6f, 62.6f)), module, Spleet::CHAIN_LIGHT));

    }
};

} // namespace Spleet

Model* modelSpleet = createModel<Spleet::Spleet, Spleet::SpleetWidget>("Spleet");
