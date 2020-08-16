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


struct Rotatoes4Widget : ModuleWidget {
    Rotatoes4Widget(Rotatoes<4>* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/faceplates/Rotatoes.svg")));

        // Signature
        addChild(createWidget<AriaSignature>(mm2px(Vec(1.0f, 114.5f))));

        // External
        addInput(createInput<AriaJackIn>(mm2px(Vec(3.52f, 15.9f)), module, Rotatoes<4>::EXT_SCALE_INPUT));

        // Group 1
        addParam( createParam<AriaKnob820>( mm2px(Vec(3.52f, 31.f)), module, Rotatoes<4>::ROTATO_PARAM + 0));
        addOutput(createOutput<AriaJackOut>(mm2px(Vec(3.52f, 41.f)), module, Rotatoes<4>::CV_OUTPUT    + 0));

        // Group 2
        addParam( createParam<AriaKnob820>( mm2px(Vec(3.52f, 52.f)), module, Rotatoes<4>::ROTATO_PARAM + 1));
        addOutput(createOutput<AriaJackOut>(mm2px(Vec(3.52f, 62.f)), module, Rotatoes<4>::CV_OUTPUT    + 1));

        // Group 3
        addParam( createParam<AriaKnob820>( mm2px(Vec(3.52f, 73.f)), module, Rotatoes<4>::ROTATO_PARAM + 2));
        addOutput(createOutput<AriaJackOut>(mm2px(Vec(3.52f, 83.f)), module, Rotatoes<4>::CV_OUTPUT    + 2));

        // Group 4
        addParam( createParam<AriaKnob820>( mm2px(Vec(3.52f, 94.f)), module, Rotatoes<4>::ROTATO_PARAM + 3));
        addOutput(createOutput<AriaJackOut>(mm2px(Vec(3.52f, 104.f)), module, Rotatoes<4>::CV_OUTPUT    + 3));

        // Screws
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    }
};

} // Namespace Controllers

Model* modelRotatoes4 = createModel<Controllers::Rotatoes<4>, Controllers::Rotatoes4Widget>("Rotatoes4");
