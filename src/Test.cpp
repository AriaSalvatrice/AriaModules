/*  Copyright (C) 2019-2020 Aria Salvatrice
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/
#include "plugin.hpp"
// #include <quickjs/quickjs.h>
#include "javascript.hpp"
#include "javascript-libraries.hpp"

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
    // JSRuntime *rt = NULL;
    // JSContext *ctx = NULL;
    // Javascript::Runtime js;
    
    Test() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        // Javascript::Runtime js;

        // js.evaluateString(JavascriptLibraries::TONALJS);
        // js.evaluateString(JavascriptLibraries::TOKENIZE);
        // js.evaluateString(JavascriptLibraries::TOVOCT);
        // js.evaluateString(JavascriptLibraries::PARSEASLEADSHEET);
        // js.evaluateString(JavascriptLibraries::LEADSHEETTOQQQQ);
        // js.evaluateString("results = leadsheetToQqqq('C D7 Esus2')");
        // const char* results = js.readVariableAsChar("results");
        // DEBUG("JS results = %s", results);

        // js.evaluateString("progression = Tonal.Progression.fromRomanNumerals('C', ['IMaj7', 'IIm7', 'V7'])");
        // // [6.049 debug src/Test.cpp:37] JS progression = CMaj7,Dm7,G7

        // js.evaluateString("number = '5' + '0'");
        // int32_t number = js.readVariableAsInt32("number");
        // DEBUG("JS number = %d", number);
        // // [6.049 debug src/Test.cpp:41] JS number = 50
        
    }

    ~Test(){

    }

    void process(const ProcessArgs& args) override {

    }
};


struct TestWidget : ModuleWidget {
    TestWidget(Test* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/faceplates/Test.svg")));
        
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
                
        for (int i = 0; i < 12; i++) {
            addInput(createInput<AriaJackIn>(mm2px(Vec(10.0, 8.0 + i * 10.0)), module, Test::TEST_INPUT + i));
            addOutput(createOutput<AriaJackOut>(mm2px(Vec(20.0, 8.0 + i * 10.0)), module, Test::TEST_OUTPUT + i));
        }

    }
};

Model* modelTest = createModel<Test, TestWidget>("Test");
