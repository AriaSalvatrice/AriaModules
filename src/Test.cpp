/*  Copyright (C) 2019-2020 Aria Salvatrice
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/
#include "plugin.hpp"

// This module is to make all sorts of tests without having to recompile too much or deal with complex code interactions.

struct Test : Module {
    enum ParamIds {
        ENUMS(TEST_PARAM, 12),
        NUM_PARAMS
    };
    enum InputIds {
        ENUMS(TEST_INPUT, 12),
        NUM_INPUTS
    };
    enum OutputIds {
        ENUMS(TEST_OUTPUT, 12),
        NUM_OUTPUTS
    };
    enum LightIds {
        ENUMS(TEST_LIGHT, 12),
        NUM_LIGHTS
    };

    
    Test() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        
    }

    ~Test(){

    }

    void process(const ProcessArgs& args) override {
        lights[TEST_LIGHT + 2].setBrightness(1.f);
        lights[TEST_LIGHT + 5].setBrightness(1.f);
    }
};


struct TestWidget : ModuleWidget {

    TestWidget(Test* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/faceplates/Test.svg")));
        
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
                
        for (size_t i = 0; i < 12; i++) {
            addChild(W::createLitInput(mm2px(Vec(10.f, 8.0 + i * 10.f)), module, Test::TEST_INPUT + i, Test::TEST_LIGHT + i));
            addChild(W::createLitOutput(mm2px(Vec(10.f, 8.0 + i * 10.f)), module, Test::TEST_INPUT + i, Test::TEST_LIGHT + i));
            addOutput(createOutput<W::JackOut>(mm2px(Vec(20.f, 8.f + i * 10.f)), module, Test::TEST_OUTPUT + i));
            addChild(createLight<AriaOutputLight>(mm2px(Vec(30.f, 8.f + i * 10.f)), module, Test::TEST_LIGHT + i));
        }

    }
};

Model* modelTest = createModel<Test, TestWidget>("Test");
