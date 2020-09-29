/*  Copyright (C) 2019-2020 Aria Salvatrice
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
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


struct SplortWidget : ModuleWidget {
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
        addInput(createInputCentered<W::JackIn>(mm2px(Vec(12.7, 20.0)), module, Splort::POLY_INPUT));

        // Split outputs with lights
        addChild(createLightCentered<AriaOutputLight>(mm2px(Vec(7.62, 29.5)),  module, Splort::SPLIT_LIGHT + 0));
        addChild(createLightCentered<AriaOutputLight>(mm2px(Vec(7.62, 37.5)),  module, Splort::SPLIT_LIGHT + 1));
        addChild(createLightCentered<AriaOutputLight>(mm2px(Vec(7.62, 45.5)),  module, Splort::SPLIT_LIGHT + 2));
        addChild(createLightCentered<AriaOutputLight>(mm2px(Vec(7.62, 53.5)),  module, Splort::SPLIT_LIGHT + 3));
        addChild(createLightCentered<AriaOutputLight>(mm2px(Vec(7.62, 61.5)),  module, Splort::SPLIT_LIGHT + 4));
        addChild(createLightCentered<AriaOutputLight>(mm2px(Vec(7.62, 69.5)),  module, Splort::SPLIT_LIGHT + 5));
        addChild(createLightCentered<AriaOutputLight>(mm2px(Vec(7.62, 77.5)),  module, Splort::SPLIT_LIGHT + 6));
        addChild(createLightCentered<AriaOutputLight>(mm2px(Vec(7.62, 85.5)),  module, Splort::SPLIT_LIGHT + 7));
        addChild(createLightCentered<AriaOutputLight>(mm2px(Vec(17.78, 29.5)), module, Splort::SPLIT_LIGHT + 8));
        addChild(createLightCentered<AriaOutputLight>(mm2px(Vec(17.78, 37.5)), module, Splort::SPLIT_LIGHT + 9));
        addChild(createLightCentered<AriaOutputLight>(mm2px(Vec(17.78, 45.5)), module, Splort::SPLIT_LIGHT + 10));
        addChild(createLightCentered<AriaOutputLight>(mm2px(Vec(17.78, 53.5)), module, Splort::SPLIT_LIGHT + 11));
        addChild(createLightCentered<AriaOutputLight>(mm2px(Vec(17.78, 61.5)), module, Splort::SPLIT_LIGHT + 12));
        addChild(createLightCentered<AriaOutputLight>(mm2px(Vec(17.78, 69.5)), module, Splort::SPLIT_LIGHT + 13));
        addChild(createLightCentered<AriaOutputLight>(mm2px(Vec(17.78, 77.5)), module, Splort::SPLIT_LIGHT + 14));
        addChild(createLightCentered<AriaOutputLight>(mm2px(Vec(17.78, 85.5)), module, Splort::SPLIT_LIGHT + 15));
        
        addOutput(createOutputCentered<AriaJackTransparent>(mm2px(Vec(7.62, 29.5)),  module, Splort::SPLIT_OUTPUT + 0));
        addOutput(createOutputCentered<AriaJackTransparent>(mm2px(Vec(7.62, 37.5)),  module, Splort::SPLIT_OUTPUT + 1));
        addOutput(createOutputCentered<AriaJackTransparent>(mm2px(Vec(7.62, 45.5)),  module, Splort::SPLIT_OUTPUT + 2));
        addOutput(createOutputCentered<AriaJackTransparent>(mm2px(Vec(7.62, 53.5)),  module, Splort::SPLIT_OUTPUT + 3));
        addOutput(createOutputCentered<AriaJackTransparent>(mm2px(Vec(7.62, 61.5)),  module, Splort::SPLIT_OUTPUT + 4));
        addOutput(createOutputCentered<AriaJackTransparent>(mm2px(Vec(7.62, 69.5)),  module, Splort::SPLIT_OUTPUT + 5));
        addOutput(createOutputCentered<AriaJackTransparent>(mm2px(Vec(7.62, 77.5)),  module, Splort::SPLIT_OUTPUT + 6));
        addOutput(createOutputCentered<AriaJackTransparent>(mm2px(Vec(7.62, 85.5)),  module, Splort::SPLIT_OUTPUT + 7));
        addOutput(createOutputCentered<AriaJackTransparent>(mm2px(Vec(17.78, 29.5)), module, Splort::SPLIT_OUTPUT + 8));
        addOutput(createOutputCentered<AriaJackTransparent>(mm2px(Vec(17.78, 37.5)), module, Splort::SPLIT_OUTPUT + 9));
        addOutput(createOutputCentered<AriaJackTransparent>(mm2px(Vec(17.78, 45.5)), module, Splort::SPLIT_OUTPUT + 10));
        addOutput(createOutputCentered<AriaJackTransparent>(mm2px(Vec(17.78, 53.5)), module, Splort::SPLIT_OUTPUT + 11));
        addOutput(createOutputCentered<AriaJackTransparent>(mm2px(Vec(17.78, 61.5)), module, Splort::SPLIT_OUTPUT + 12));
        addOutput(createOutputCentered<AriaJackTransparent>(mm2px(Vec(17.78, 69.5)), module, Splort::SPLIT_OUTPUT + 13));
        addOutput(createOutputCentered<AriaJackTransparent>(mm2px(Vec(17.78, 77.5)), module, Splort::SPLIT_OUTPUT + 14));
        addOutput(createOutputCentered<AriaJackTransparent>(mm2px(Vec(17.78, 85.5)), module, Splort::SPLIT_OUTPUT + 15));
        
        // Sort button
        addParam(createParamCentered<AriaPushButton700>(mm2px(Vec(12.7, 95.0)), module, Splort::SORT_PARAM));

        // Link jacks with lights
        addChild(createLightCentered<AriaInputLight>(mm2px(Vec(5.62, 109.0)), module, Splort::LINK_IN_LIGHT));
        addChild(createLightCentered<AriaOutputLight>(mm2px(Vec(19.78, 109.0)), module, Splort::LINK_OUT_LIGHT));
        
        addInput(createInputCentered<AriaJackTransparent>(mm2px(Vec(5.62, 109.0)), module, Splort::LINK_INPUT));
        addOutput(createOutputCentered<AriaJackTransparent>(mm2px(Vec(19.78, 109.0)), module, Splort::LINK_OUTPUT));
        
    }
};

} // namespace Splort

Model* modelSplort = createModel<Splort::Splort, Splort::SplortWidget>("Splort");
