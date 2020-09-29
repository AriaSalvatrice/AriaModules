/*  Copyright (C) 2019-2020 Aria Salvatrice
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/


// Warning - this module was created with very little C++ experience, and features were 
// added to it later without regard for code quality. This is maintained exploratory code, not good design.


#include "plugin.hpp"

namespace Splirge {

struct Splirge : Module {
    enum ParamIds {
        SORT_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        POLY_INPUT,
        ENUMS(MERGE_INPUT, 4),
        NUM_INPUTS
    };
    enum OutputIds {
        POLY_OUTPUT,
        ENUMS(SPLIT_OUTPUT, 4),
        NUM_OUTPUTS
    };
    enum LightIds {
        POLY_LIGHT,
        ENUMS(SPLIT_LIGHT, 4),
        CHAIN_LIGHT,
        NUM_LIGHTS
    };
    
    dsp::ClockDivider ledDivider;

    Splirge() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        ledDivider.setDivision(4096);
        configParam(SORT_PARAM, 0.f, 1.f, 0.f, "Sort voltages on both banks");
    }
    
    // Merge without sorting, faster
    void merge() {

        // Don't waste time if there's no output connected
        if (!outputs[POLY_OUTPUT].isConnected()) return;

        int lastMergeChannel = 0;
        for (size_t i = 0; i < 4; i++) {
            if (inputs[MERGE_INPUT + i].isConnected()) {
                outputs[POLY_OUTPUT].setVoltage(inputs[MERGE_INPUT + i].getVoltage(), i);
                lastMergeChannel = i+1;
            } else {
                outputs[POLY_OUTPUT].setVoltage(0, i);
            }
        }
        outputs[POLY_OUTPUT].setChannels(lastMergeChannel);
    }

    // Split without sorting, faster
    void split() {

        // Don't waste time if there's no output connected
        if (   !outputs[SPLIT_OUTPUT].isConnected()     && !outputs[SPLIT_OUTPUT + 1].isConnected()
            && !outputs[SPLIT_OUTPUT + 2].isConnected() && !outputs[SPLIT_OUTPUT + 3].isConnected()) {
             return;
        }

        for (size_t i = 0; i < 4; i++)
            outputs[SPLIT_OUTPUT + i].setVoltage( (inputs[POLY_INPUT].isConnected()) ? inputs[POLY_INPUT].getVoltage(i) : inputs[MERGE_INPUT + i].getVoltage());
    }

    // Merge with sorting
    void mergeSort() {

        // Don't waste time if there's no output connected
        if (!outputs[POLY_OUTPUT].isConnected()) return;

        std::array<float, 4> mergedVoltages;
        size_t connected = 0;

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
            outputs[POLY_OUTPUT].setVoltage(mergedVoltages[i], i);
        outputs[POLY_OUTPUT].setChannels(connected);
    }


    // Split with sorting
    void splitSort() {

        std::array<float, 4> splitVoltages;	
        size_t connected = 0;

        // How many connected inputs?
        if (inputs[POLY_INPUT].isConnected()) {
            connected = inputs[POLY_INPUT].getChannels();
        } else { // Internal default wiring
            for (size_t i = 0; i < 4; i++)
                connected = (inputs[MERGE_INPUT + i].isConnected()) ? i + 1 : connected;
        }
        
        // Fill array
        for (size_t i = 0; i < 4; i++)
            if (i < connected)
                splitVoltages[i] = (inputs[POLY_INPUT].isConnected()) ? inputs[POLY_INPUT].getVoltage(i) : inputs[MERGE_INPUT + i].getVoltage();
        
        // Sort and output
        std::sort(splitVoltages.begin(), splitVoltages.begin() + connected);
        for (size_t i = 0; i < 4; i++)
            outputs[SPLIT_OUTPUT + i].setVoltage(splitVoltages[i]);	
    }
    
    void updateLeds() {
        // Chain light
        lights[CHAIN_LIGHT].setBrightness( (inputs[POLY_INPUT].isConnected())? 0.f : 1.f);

        // Merge output
        int mergeInputCount = 0;
        int lastInput = 0;
        for (size_t i = 0; i < 4; i++)
            if (inputs[MERGE_INPUT + i].isConnected()){
                mergeInputCount++;
                lastInput = i;
            }
        lights[POLY_LIGHT].setBrightness( (mergeInputCount > 0) ? 1.f : 0.f);
        
        // Split outputs
        for (int i = 0; i < 4; i++) {
            if (inputs[POLY_INPUT].isConnected()) { // External wiring 
                lights[SPLIT_LIGHT + i].setBrightness( (inputs[POLY_INPUT].getChannels() > i) ? 1.f : 0.f);
            } else {  // Internal wiring
                if (params[SORT_PARAM].getValue()){ // Sorted mode
                    lights[SPLIT_LIGHT + i].setBrightness( (i <= lastInput) ? 1.f : 0.f);
                } else { // Unsorted mode
                    lights[SPLIT_LIGHT + i].setBrightness( (inputs[MERGE_INPUT + i].isConnected()) ? 1.f : 0.f);
                }
            }
        }
    }

    void process(const ProcessArgs& args) override {
        if (params[SORT_PARAM].getValue()) {
            mergeSort();
            splitSort();
        } else {
            merge();
            split();
        }	
        if (ledDivider.process())
            updateLeds();
    }
};


struct SplirgeWidget : ModuleWidget {
    SplirgeWidget(Splirge* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/faceplates/Splirge.svg")));
        
        // Signature 
        addChild(createWidget<W::Signature>(mm2px(Vec(1.f, 114.5f))));

        // Screws
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        
        // Pushbutton
        addParam(createParam<AriaPushButton500>(mm2px(Vec(1.0, 62.8)), module, Splirge::SORT_PARAM));

        // Jacks, top to bottom.
        addInput(createInputCentered<W::JackIn>(mm2px(Vec(7.62, 20.0)), module, Splirge::MERGE_INPUT + 0));
        addInput(createInputCentered<W::JackIn>(mm2px(Vec(7.62, 28.0)), module, Splirge::MERGE_INPUT + 1));
        addInput(createInputCentered<W::JackIn>(mm2px(Vec(7.62, 36.0)), module, Splirge::MERGE_INPUT + 2));
        addInput(createInputCentered<W::JackIn>(mm2px(Vec(7.62, 44.0)), module, Splirge::MERGE_INPUT + 3));
        
        addChild(createLightCentered<AriaOutputLight>(mm2px(Vec(7.62, 54.0)), module, Splirge::POLY_LIGHT));
        addOutput(createOutputCentered<AriaJackTransparent>(mm2px(Vec(7.62, 54.0)), module, Splirge::POLY_OUTPUT));
        
        addInput(createInputCentered<W::JackIn>(mm2px(Vec(7.62, 72.5)), module, Splirge::POLY_INPUT));
        
        addChild(createLightCentered<AriaOutputLight>(mm2px(Vec(7.62, 85.0)), module, Splirge::SPLIT_LIGHT  + 0));
        addChild(createLightCentered<AriaOutputLight>(mm2px(Vec(7.62, 93.0)), module, Splirge::SPLIT_LIGHT  + 1));
        addChild(createLightCentered<AriaOutputLight>(mm2px(Vec(7.62, 101.0)), module, Splirge::SPLIT_LIGHT + 2));
        addChild(createLightCentered<AriaOutputLight>(mm2px(Vec(7.62, 109.0)), module, Splirge::SPLIT_LIGHT + 3));
        
        addOutput(createOutputCentered<AriaJackTransparent>(mm2px(Vec(7.62,  85.0)), module, Splirge::SPLIT_OUTPUT   + 0));
        addOutput(createOutputCentered<AriaJackTransparent>(mm2px(Vec(7.62,  93.0)), module, Splirge::SPLIT_OUTPUT   + 1));
        addOutput(createOutputCentered<AriaJackTransparent>(mm2px(Vec(7.62, 101.0)), module, Splirge::SPLIT_OUTPUT   + 2));
        addOutput(createOutputCentered<AriaJackTransparent>(mm2px(Vec(7.62, 109.0)), module, Splirge::SPLIT_OUTPUT   + 3));
        
        // Chain light
        addChild(createLightCentered<SmallLight<InputLight>>(mm2px(Vec(13.6, 69.0)), module, Splirge::CHAIN_LIGHT));

    }
};

} // namespace Splirge

Model* modelSplirge = createModel<Splirge::Splirge, Splirge::SplirgeWidget>("Splirge");
