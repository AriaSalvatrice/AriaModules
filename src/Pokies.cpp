/*  Copyright (C) 2019-2020 Aria Salvatrice
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

// 4 buttons for performance. Code is in great part taken from Rotatoes.cpp

#include "plugin.hpp"

namespace Pokies {

// Nope, no audio rate option provided until someone makes a strong argument why they need it.
const int PROCESSDIVIDER = 32;

template <size_t BUTTONS>
struct Pokies : Module {
    enum ParamIds {
        ENUMS(POKIE_PARAM, BUTTONS),
        NUM_PARAMS
    };
    enum InputIds {
        GLOBAL_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        ENUMS(CV_OUTPUT, BUTTONS),
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    float min[BUTTONS];
    float max[BUTTONS];
    std::array<bool, BUTTONS> momentary;
    dsp::ClockDivider processDivider;

    
    Pokies() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        for(size_t i = 0; i < BUTTONS; i++){
            configParam(POKIE_PARAM + i, 0.f, 1.f, 0.f, "Pokie " + std::to_string(i + 1));
            min[i] = 0.f;
            max[i] = 10.f;
            momentary[i] = true;
        }

        processDivider.setDivision(PROCESSDIVIDER);
    }


    json_t* dataToJson() override {
        json_t* rootJ = json_object();

        json_t *minJ = json_array();
        for (size_t i = 0; i < BUTTONS; i++) json_array_insert_new(minJ, i, json_real(min[i]));
        json_object_set_new(rootJ, "min", minJ);
        
        json_t *maxJ = json_array();
        for (size_t i = 0; i < BUTTONS; i++) json_array_insert_new(maxJ, i, json_real(max[i]));
        json_object_set_new(rootJ, "max", maxJ);

        json_t *momentaryJ = json_array();
        for (size_t i = 0; i < BUTTONS; i++) json_array_insert_new(momentaryJ, i, json_boolean(momentary[i]));
        json_object_set_new(rootJ, "momentary", momentaryJ);

        return rootJ;
    }


    void dataFromJson(json_t* rootJ) override {
        json_t *minJ = json_object_get(rootJ, "min");
        if (minJ) {
            for (size_t i = 0; i < BUTTONS; i++) {
                json_t *minValueJ = json_array_get(minJ, i);
                if (minValueJ) min[i] = json_real_value(minValueJ);
            }
        }

        json_t *maxJ = json_object_get(rootJ, "max");
        if (maxJ) {
            for (size_t i = 0; i < BUTTONS; i++) {
                json_t *maxValueJ = json_array_get(maxJ, i);
                if (maxValueJ) max[i] = json_real_value(maxValueJ);
            }
        }

        json_t *momentaryJ = json_object_get(rootJ, "momentary");
        if (momentaryJ) {
            for (size_t i = 0; i < BUTTONS; i++) {
                json_t *momentaryValueJ = json_array_get(momentaryJ, i);
                if (momentaryValueJ) momentary[i] = json_boolean_value(momentaryValueJ);
            }
        }
    }

    
    void process(const ProcessArgs& args) override {

    }
};


// Add a margin to my normal button, so the square that shows it's bound to MIDI is offset a bit.
// A black placeholder square is added to the faceplate. Positioning of the rectangle is yolo'd.
// FIXME: Momentary/Latch doesn't work with MIDI. I need a separate widget.
struct Pokie : AriaPushButton820Pink {
    Pokie() {
        AriaPushButton820Pink();
        box.size.x += mm2px(1.35f);
        box.size.y += mm2px(0.71f);
    }
};


struct PokiesWidget : ModuleWidget {

    void drawPokie(Pokies<4>* module, float y, int num) {
        addParam(createParam<Pokie>(mm2px(Vec(3.52f, y)), module, Pokies<4>::POKIE_PARAM + num));
        addOutput(createOutput<AriaJackOut>(mm2px(Vec(3.52f, y + 10.f)), module, Pokies<4>::CV_OUTPUT + num));
    }

    PokiesWidget(Pokies<4>* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/faceplates/Pokies.svg")));
        
        // Signature
        addChild(createWidget<AriaSignature>(mm2px(Vec(1.0f, 114.5f))));

        // Global Input
        addInput(createInput<AriaJackIn>(mm2px(Vec(3.52f, 15.9f)), module, Pokies<4>::GLOBAL_INPUT));

        // Pokies
        drawPokie(module, 31.f, 0);
        drawPokie(module, 52.f, 1);
        drawPokie(module, 73.f, 2);
        drawPokie(module, 94.f, 3);

        // Screws
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    }
};

} // namespace Pokies

Model* modelPokies4 = createModel<Pokies::Pokies<4>, Pokies::PokiesWidget>("Pokies4");
