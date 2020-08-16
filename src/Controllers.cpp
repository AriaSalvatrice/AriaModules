/*  Copyright (C) 2019-2020 Aria Salvatrice
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

// Small controllers.
// Rotatoes = knobs
// Faders and buttons planned next.

// Nope, no audio rate option provided until someone makes a strong argument why they need it.
const int PROCESSDIVIDER = 32;

#include "plugin.hpp"

namespace Controllers {

template <size_t KNOBS>
struct Rotatoes : Module {
    enum ParamIds {
        ENUMS(ROTATO_PARAM, KNOBS),
        NUM_PARAMS
    };
    enum InputIds {
        EXT_SCALE_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        ENUMS(CV_OUTPUT, KNOBS),
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };
    
    Rotatoes() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    }
    
    void process(const ProcessArgs& args) override {

    }
};

// Add a margin to my normal knob, so the square that shows it's bound to MIDI is offset a bit.
// A black placeholder square is added to the faceplate. Position numbers are yolo'd. 
struct AriaKnob820Margin : AriaKnob820 {
    AriaKnob820Margin() {
        AriaKnob820();
        box.size.x += 4.f;
        box.size.y += 2.1f;
    }
};


struct Rotatoes4Widget : ModuleWidget {

    void drawRotato(Rotatoes<4>* module, float y, int num) {
        addParam(createParam<AriaKnob820Margin>(mm2px(Vec(3.52f, y)), module, Rotatoes<4>::ROTATO_PARAM + num));
        addOutput(createOutput<AriaJackOut>(mm2px(Vec(3.52f, y + 10.f)), module, Rotatoes<4>::CV_OUTPUT + num));
    }

    Rotatoes4Widget(Rotatoes<4>* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/faceplates/Rotatoes.svg")));

        // Signature
        addChild(createWidget<AriaSignature>(mm2px(Vec(1.0f, 114.5f))));

        // External
        addInput(createInput<AriaJackIn>(mm2px(Vec(3.52f, 15.9f)), module, Rotatoes<4>::EXT_SCALE_INPUT));

        // Rotatoes
        drawRotato(module, 31.f, 0);
        drawRotato(module, 52.f, 1);
        drawRotato(module, 73.f, 2);
        drawRotato(module, 94.f, 3);

        // Screws
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    }
};

} // Namespace Controllers

Model* modelRotatoes4 = createModel<Controllers::Rotatoes<4>, Controllers::Rotatoes4Widget>("Rotatoes4");
