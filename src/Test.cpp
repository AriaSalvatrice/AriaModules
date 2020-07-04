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

    }
};



struct BraidsDisplay : TransparentWidget {
	Test* module;
	std::shared_ptr<Font> font;

	BraidsDisplay() {
		font = APP->window->loadFont(asset::plugin(pluginInstance, "res/dseg/DSEG14ClassicMini-Italic.ttf"));
	}

	void draw(const DrawArgs& args) override {
		nvgFontSize(args.vg, 20);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, 2.0);
		nvgFillColor(args.vg, nvgRGB(0x0b, 0x52, 0x5d));
		nvgText(args.vg, 0, 0, "~~~~~~~~~~", NULL);
		nvgFillColor(args.vg, nvgRGB(0xc1, 0xf0, 0xf2));
		nvgText(args.vg, 0, 0, "hi!gamers", NULL);
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
            // addInput(createInput<AriaJackIn>(mm2px(Vec(10.0, 8.0 + i * 10.0)), module, Test::TEST_INPUT + i));
            // addOutput(createOutput<AriaJackOut>(mm2px(Vec(20.0, 8.0 + i * 10.0)), module, Test::TEST_OUTPUT + i));
        }

        BraidsDisplay* display = new BraidsDisplay();
        display->box.pos = mm2px(Vec(5.0, 80.0));
        display->box.size = mm2px(Vec(31.0, 10.0));
        display->module = module;
        addChild(display);

    }
};

Model* modelTest = createModel<Test, TestWidget>("Test");
