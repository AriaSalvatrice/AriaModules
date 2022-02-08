/*  Copyright (C) 2019-2020 Aria Salvatrice
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/


// Warning - this module was created with very little C++ experience, and features were 
// added to it later without regard for code quality. This is maintained exploratory code, not good design.


#include "plugin.hpp"

namespace Splort {

struct Splort : Module {
    enum ParamIds {
        SORT_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        POLY_INPUT,
        LINK_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        ENUMS(SPLIT_OUTPUT, 16),
        LINK_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        ENUMS(SPLIT_LIGHT, 16),
        LINK_IN_LIGHT,
        LINK_OUT_LIGHT,
        NUM_LIGHTS
    };
    
    dsp::ClockDivider ledDivider;

    Splort() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        ledDivider.setDivision(256);
        configParam(SORT_PARAM, 0.f, 1.f, 0.f, "Sort voltages");
        configInput(POLY_INPUT, "Poly");
        configInput(LINK_INPUT, "Link");
        for (int i = 0; i < 16; i++)
            configOutput(SPLIT_OUTPUT + i, string::f("Channel %d", i + 1));
        configOutput(LINK_OUTPUT, "Link");
    }
    
    // Split without sorting, faster
    void split() {
        for (size_t i = 0; i < 16; i++)
            outputs[SPLIT_OUTPUT + i].setVoltage(inputs[POLY_INPUT].getVoltage(i));
    }
    
    // Split with sorting, and send Link output
    void splitSortLink() {
        std::array<std::array<float, 2>, 16> splitVoltages;	
        size_t connected = 0;

        // How many connected inputs?
        connected = inputs[POLY_INPUT].getChannels();
        if (! inputs[LINK_INPUT].isConnected())
            outputs[LINK_OUTPUT].setChannels(connected);

        // Fill array
        for (size_t i = 0; i < 16; i++) {
            if (i < connected) {
                splitVoltages[i][0] = inputs[POLY_INPUT].getVoltage(i);
                splitVoltages[i][1] = (inputs[LINK_INPUT].isConnected()) ? inputs[LINK_INPUT].getVoltage(i) : (i + 1.f) * 0.1f;
            } else {
                splitVoltages[i][0] = 0.0f;
                splitVoltages[i][1] = (inputs[LINK_INPUT].isConnected()) ? inputs[LINK_INPUT].getVoltage(i) : 0.f;
            }
        }
        
        // Sort
        if (inputs[LINK_INPUT].isConnected()) { // Sort by 2nd member of array.
            std::sort(splitVoltages.begin(), splitVoltages.begin() + connected, [](const std::array<float, 2> &left, const std::array<float, 2> &right) {
                if (left[1] == 0.f)
                    return false;
                return left[1] < right[1];
            });	
        } else { // Sort by 1st member
            std::sort(splitVoltages.begin(), splitVoltages.begin() + connected);
        }
        
        // Output
        for (size_t i = 0; i < 16; i++) {
            outputs[SPLIT_OUTPUT + i].setVoltage(splitVoltages[i][0]);
            if (! inputs[LINK_INPUT].isConnected())
                outputs[LINK_OUTPUT].setVoltage(splitVoltages[i][1], i);
        }
    }
    
    void chainLink() {
        if (inputs[LINK_INPUT].isConnected()) {
            outputs[LINK_OUTPUT].setChannels(inputs[LINK_INPUT].getChannels());
            for (size_t i = 0; i < 16; i++)
                outputs[LINK_OUTPUT].setVoltage(inputs[LINK_INPUT].getVoltage(i), i);
        } else {
            if (! params[SORT_PARAM].getValue())
                outputs[LINK_OUTPUT].setChannels(0);
        }
    }
    
    void updateLeds() {
        if ( (params[SORT_PARAM].getValue()) or (inputs[LINK_INPUT].isConnected()) ) {
            lights[LINK_IN_LIGHT].setBrightness(1.f);
            lights[LINK_OUT_LIGHT].setBrightness(1.f);
        } else {
            lights[LINK_IN_LIGHT].setBrightness(0.f);
            lights[LINK_OUT_LIGHT].setBrightness(0.f);
        }	
        for (int i = 0; i < 16; i++)
            lights[SPLIT_LIGHT + i].setBrightness( (inputs[POLY_INPUT].getChannels() > i) ? 1.f : 0.f);
    }
    
    void process(const ProcessArgs& args) override {
        (params[SORT_PARAM].getValue()) ? splitSortLink() : split();
        chainLink(); // Chain link inputs, whether sorting or not
        if (ledDivider.process())
            updateLeds();
    }
};


struct SplortWidget : W::ModuleWidget {
    SplortWidget(Splort* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/faceplates/Splort.svg")));
        
        // Signature 
        addChild(createWidget<W::Signature>(mm2px(Vec(5.9f, 114.5f))));

        // Screws
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Split input
        addStaticInput(mm2px(Vec(8.6f, 15.9f)), module, Splort::POLY_INPUT);

        // Split outputs with lights
        addDynamicOutput(mm2px(Vec( 3.52f, 25.4f)), module, Splort::SPLIT_OUTPUT +  0, Splort::SPLIT_LIGHT +  0);
        addDynamicOutput(mm2px(Vec( 3.52f, 33.4f)), module, Splort::SPLIT_OUTPUT +  1, Splort::SPLIT_LIGHT +  1);
        addDynamicOutput(mm2px(Vec( 3.52f, 41.4f)), module, Splort::SPLIT_OUTPUT +  2, Splort::SPLIT_LIGHT +  2);
        addDynamicOutput(mm2px(Vec( 3.52f, 49.4f)), module, Splort::SPLIT_OUTPUT +  3, Splort::SPLIT_LIGHT +  3);
        addDynamicOutput(mm2px(Vec( 3.52f, 57.4f)), module, Splort::SPLIT_OUTPUT +  4, Splort::SPLIT_LIGHT +  4);
        addDynamicOutput(mm2px(Vec( 3.52f, 65.4f)), module, Splort::SPLIT_OUTPUT +  5, Splort::SPLIT_LIGHT +  5);
        addDynamicOutput(mm2px(Vec( 3.52f, 73.4f)), module, Splort::SPLIT_OUTPUT +  6, Splort::SPLIT_LIGHT +  6);
        addDynamicOutput(mm2px(Vec( 3.52f, 81.4f)), module, Splort::SPLIT_OUTPUT +  7, Splort::SPLIT_LIGHT +  7);
        addDynamicOutput(mm2px(Vec(13.68f, 25.4f)), module, Splort::SPLIT_OUTPUT +  8, Splort::SPLIT_LIGHT +  8);
        addDynamicOutput(mm2px(Vec(13.68f, 33.4f)), module, Splort::SPLIT_OUTPUT +  9, Splort::SPLIT_LIGHT +  9);
        addDynamicOutput(mm2px(Vec(13.68f, 41.4f)), module, Splort::SPLIT_OUTPUT + 10, Splort::SPLIT_LIGHT + 10);
        addDynamicOutput(mm2px(Vec(13.68f, 49.4f)), module, Splort::SPLIT_OUTPUT + 11, Splort::SPLIT_LIGHT + 11);
        addDynamicOutput(mm2px(Vec(13.68f, 57.4f)), module, Splort::SPLIT_OUTPUT + 12, Splort::SPLIT_LIGHT + 12);
        addDynamicOutput(mm2px(Vec(13.68f, 65.4f)), module, Splort::SPLIT_OUTPUT + 13, Splort::SPLIT_LIGHT + 13);
        addDynamicOutput(mm2px(Vec(13.68f, 73.4f)), module, Splort::SPLIT_OUTPUT + 14, Splort::SPLIT_LIGHT + 14);
        addDynamicOutput(mm2px(Vec(13.68f, 81.4f)), module, Splort::SPLIT_OUTPUT + 15, Splort::SPLIT_LIGHT + 15);
        
        // Sort button
        addParam(createParam<W::ReducedButton>(mm2px(Vec(8.6f, 90.9f)), module, Splort::SORT_PARAM));

        // Link jacks with lights
        addDynamicInput(mm2px(Vec(1.52f, 104.9f)), module, Splort::LINK_INPUT, Splort::LINK_IN_LIGHT);
        addDynamicOutput(mm2px(Vec(15.68f, 104.9f)), module, Splort::LINK_OUTPUT, Splort::LINK_OUT_LIGHT);
        
    }
};

} // namespace Splort

Model* modelSplort = createModel<Splort::Splort, Splort::SplortWidget>("Splort");
