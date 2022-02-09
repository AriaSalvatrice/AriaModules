/*  Copyright (C) 2019-2020 Aria Salvatrice
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/


// Warning - this module was created with very little C++ experience, and features were 
// added to it later without regard for code quality. This is maintained exploratory code, not good design.


#include "plugin.hpp"

namespace Smerge{

struct Smerge : Module {
    enum ParamIds {
        SORT_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        LINK_INPUT,
        ENUMS(MERGE_INPUT, 16),
        NUM_INPUTS
    };
    enum OutputIds {
        POLY_OUTPUT,
        LINK_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        LINK_IN_LIGHT,
        LINK_OUT_LIGHT,
        NUM_LIGHTS
    };
    
    dsp::ClockDivider ledDivider;

    Smerge() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        ledDivider.setDivision(256);
        configParam(SORT_PARAM, 0.f, 1.f, 0.f, "Sort voltages");
        configInput(LINK_INPUT, "Link");
        for (int i = 0; i < 16; i++)
            configInput(MERGE_INPUT + i, string::f("Channel %d", i + 1));
        configOutput(POLY_OUTPUT, "Poly");
        configOutput(LINK_OUTPUT, "Link");
    }
    
    // Merge without sorting, faster
    void merge() {

        // Don't waste time if there's no output connected
        if (!outputs[POLY_OUTPUT].isConnected()) return;

        int lastMergeChannel = 0;
        for (size_t i = 0; i < 16; i++) {
            if (inputs[MERGE_INPUT + i].isConnected()) {
                outputs[POLY_OUTPUT].setVoltage(inputs[MERGE_INPUT + i].getVoltage(), i);
                lastMergeChannel = i+1;
            } else {
                outputs[POLY_OUTPUT].setVoltage(0, i);
            }
        }
        outputs[POLY_OUTPUT].setChannels(lastMergeChannel);
    }
    
    // Merge with sorting, and send Link output
    void mergeSortLink() {
        std::array<std::array<float, 2>, 16> mergedVoltages;	
        size_t connected = 0;
        
        if (inputs[LINK_INPUT].isConnected()) {
            // Link input
            bool lastFound = false;
            for (int i = 15; i >= 0; i--) {
                mergedVoltages[i][0] = inputs[MERGE_INPUT + i].getVoltage();
                mergedVoltages[i][1] = inputs[LINK_INPUT].getVoltage(i);
                if (inputs[MERGE_INPUT + i].getVoltage() != 0.f)
                    lastFound = true;
                if ((inputs[MERGE_INPUT + i].getVoltage() == 0.f) and (!lastFound))
                    connected = i;
            }
            std::sort(mergedVoltages.begin(), mergedVoltages.begin() + connected, [](const std::array<float, 2> &left, const std::array<float, 2> &right) {
                if (left[1] == 0.f)
                    return false;
                return left[1] < right[1];
            });	
        } else {
            // No link input
            for (size_t i = 0; i < 16; i++) {
                if (inputs[MERGE_INPUT + i].isConnected()) {
                    mergedVoltages[i][0] = inputs[MERGE_INPUT + i].getVoltage();
                    mergedVoltages[i][1] = (i + 1.f) * 0.1f;
                    connected = i + 1;
                } else {
                    mergedVoltages[i][0] = 0.f;
                    mergedVoltages[i][1] = 0.f;
                }
            }
            std::sort(mergedVoltages.begin(), mergedVoltages.begin() + connected);		
        }
        
        // Send to poly output
        for (size_t i = 0; i < connected; i++) {
            outputs[POLY_OUTPUT].setVoltage(mergedVoltages[i][0], i);
        }
        outputs[POLY_OUTPUT].setChannels(connected);
        
        // Send to link output
        if (! inputs[LINK_INPUT].isConnected()) {
            outputs[LINK_OUTPUT].setChannels(connected);
            for (size_t i = 0; i < 16; i++) {
                outputs[LINK_OUTPUT].setVoltage(mergedVoltages[i][1], i);
            }
        }
    }
    
    void chainLink() {
        if (inputs[LINK_INPUT].isConnected()) {
            outputs[LINK_OUTPUT].setChannels(inputs[LINK_INPUT].getChannels());
            for (size_t i = 0; i < 16; i++) {
                outputs[LINK_OUTPUT].setVoltage(inputs[LINK_INPUT].getVoltage(i), i);
            }
        } else {
            if (! params[SORT_PARAM].getValue()) {
                outputs[LINK_OUTPUT].setChannels(0);
            }
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
    }


    void process(const ProcessArgs& args) override {
        (params[SORT_PARAM].getValue()) ? mergeSortLink() : merge();
        chainLink(); // Chain link inputs, whether sorting or not
        if (ledDivider.process())
            updateLeds();
    }	
};


struct SmergeWidget : W::ModuleWidget {
    SmergeWidget(Smerge* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/faceplates/Smerge.svg")));
        
        // Signature 
        addChild(createWidget<W::Signature>(mm2px(Vec(5.9f, 114.5f))));

        // Screws
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Merge Output
        addStaticOutput(mm2px(Vec(8.6f, 15.9f)),  module, Smerge::POLY_OUTPUT);
        
        // Merge Inputs
        addStaticInput(mm2px(Vec( 3.52f, 25.4f)), module, Smerge::MERGE_INPUT + 0);
        addStaticInput(mm2px(Vec( 3.52f, 33.4f)), module, Smerge::MERGE_INPUT + 1);
        addStaticInput(mm2px(Vec( 3.52f, 41.4f)), module, Smerge::MERGE_INPUT + 2);
        addStaticInput(mm2px(Vec( 3.52f, 49.4f)), module, Smerge::MERGE_INPUT + 3);
        addStaticInput(mm2px(Vec( 3.52f, 57.4f)), module, Smerge::MERGE_INPUT + 4);
        addStaticInput(mm2px(Vec( 3.52f, 65.4f)), module, Smerge::MERGE_INPUT + 5);
        addStaticInput(mm2px(Vec( 3.52f, 73.4f)), module, Smerge::MERGE_INPUT + 6);
        addStaticInput(mm2px(Vec( 3.52f, 81.4f)), module, Smerge::MERGE_INPUT + 7);
        addStaticInput(mm2px(Vec(13.68f, 25.4f)), module, Smerge::MERGE_INPUT + 8);
        addStaticInput(mm2px(Vec(13.68f, 33.4f)), module, Smerge::MERGE_INPUT + 9);
        addStaticInput(mm2px(Vec(13.68f, 41.4f)), module, Smerge::MERGE_INPUT + 10);
        addStaticInput(mm2px(Vec(13.68f, 49.4f)), module, Smerge::MERGE_INPUT + 11);
        addStaticInput(mm2px(Vec(13.68f, 57.4f)), module, Smerge::MERGE_INPUT + 12);
        addStaticInput(mm2px(Vec(13.68f, 65.4f)), module, Smerge::MERGE_INPUT + 13);
        addStaticInput(mm2px(Vec(13.68f, 73.4f)), module, Smerge::MERGE_INPUT + 14);
        addStaticInput(mm2px(Vec(13.68f, 81.4f)), module, Smerge::MERGE_INPUT + 15);

        // Sort button
        addParam(createParam<W::ReducedButton>(mm2px(Vec(8.6f, 90.9f)), module, Smerge::SORT_PARAM));

        // Link jacks with lights
        addDynamicInput(mm2px(Vec(1.52f, 104.9f)), module, Smerge::LINK_INPUT, Smerge::LINK_IN_LIGHT);
        addDynamicOutput(mm2px(Vec(15.68f, 104.9f)), module, Smerge::LINK_OUTPUT, Smerge::LINK_OUT_LIGHT);
    }
};

} // namespace Smerge


Model* modelSmerge = createModel<Smerge::Smerge, Smerge::SmergeWidget>("Smerge");
