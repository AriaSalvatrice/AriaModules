/*  Copyright (C) 2019-2020 Aria Salvatrice
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

// Chords to scale, scales to chords. Expander connections with Qqqq.

#include "plugin.hpp"
#include "quantizer.hpp"

namespace Quale {

const int PROCESSDIVIDER = 32;

struct Quale : Module {
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        SCALE_INPUT,
        CHORD_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        SCALE_OUTPUT,
        CHORD_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        EXPANDER_IN_LIGHT,
        EXPANDER_OUT_LIGHT,
        SCALE_TO_CHORD_LIGHT,
        NUM_LIGHTS
    };

    std::array<std::array<bool, 12>, 2> leftMessages;
    std::array<bool, 12> scale;
    dsp::ClockDivider processDivider;
    
    Quale() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        processDivider.setDivision(PROCESSDIVIDER);
        leftExpander.producerMessage = &leftMessages[0];
        leftExpander.consumerMessage = &leftMessages[1];
    }

    void processScaleToChord() {
        size_t j = 0;

        // If we are an expander, the input jack is not used.
        if ((leftExpander.module and leftExpander.module->model == modelQqqq)
        ||  (leftExpander.module and leftExpander.module->model == modelQuack)
        ||  (leftExpander.module and leftExpander.module->model == modelQ)) {
            // We are an expander
            lights[EXPANDER_IN_LIGHT].setBrightness(1.f);
            lights[SCALE_TO_CHORD_LIGHT].setBrightness(0.f);
            bool *message = (bool*) leftExpander.consumerMessage;
            if (outputs[CHORD_OUTPUT].isConnected()) {
                for (size_t i = 0; i < 12; i++) {
                    if (message[i]) outputs[CHORD_OUTPUT].setVoltage((i * 1.f/12.f) , j++);
                }
                outputs[CHORD_OUTPUT].setChannels(j);
            }
        } else {
            // We are not an expander
            lights[EXPANDER_IN_LIGHT].setBrightness(0.f);
            lights[SCALE_TO_CHORD_LIGHT].setBrightness(1.f);
            if (outputs[CHORD_OUTPUT].isConnected()){
                for (int i = 0; i < inputs[SCALE_INPUT].getChannels(); i++) {
                    if (inputs[SCALE_INPUT].getVoltage(i) > 0.f) outputs[CHORD_OUTPUT].setVoltage((i * 1.f/12.f) , j++);
                }
                outputs[CHORD_OUTPUT].setChannels(j);
            }
        }
    }

    void processChordToScale() {
        // Whether we have an expander or not, the input comes from the jack
        for (size_t i = 0; i < 12; i++) scale[i] = false;
        if (inputs[CHORD_INPUT].isConnected()) {
            for (int i = 0; i < inputs[CHORD_INPUT].getChannels(); i++) {
                scale[Quantizer::quantizeToPositionInOctave(inputs[CHORD_INPUT].getVoltage(i), Quantizer::validNotesInScale(Quantizer::CHROMATIC))] = true;
            }
        }

        if ((rightExpander.module and rightExpander.module->model == modelQqqq)
        ||  (rightExpander.module and rightExpander.module->model == modelQuack)
        ||  (rightExpander.module and rightExpander.module->model == modelQ)) {
            // We have an expander
            lights[EXPANDER_OUT_LIGHT].setBrightness(1.f);
            bool *message = (bool*) rightExpander.module->leftExpander.producerMessage;			
            for (size_t i = 0; i < 12; i++) message[i] = scale[i];
            rightExpander.module->leftExpander.messageFlipRequested = true;
        } else {
            // We have no expander
            lights[EXPANDER_OUT_LIGHT].setBrightness(0.f);
        }

        if (outputs[SCALE_OUTPUT].isConnected()){
            for (size_t i = 0; i < 12; i++) outputs[SCALE_OUTPUT].setVoltage( (scale[i]) ? 10.f : 0.f, i);
            outputs[SCALE_OUTPUT].setChannels(12);
        }
    }
    
    void process(const ProcessArgs& args) override {
        if (processDivider.process()) {
            processScaleToChord();
            processChordToScale();
        }
    }
};


struct QualeWidget : ModuleWidget {
    QualeWidget(Quale* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/faceplates/Quale.svg")));

        // Signature
        addChild(createWidget<AriaSignature>(mm2px(Vec(1.0f, 114.5f))));
        
        // Screws
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Jacks
        addInput(createInput<AriaJackIn>(mm2px(Vec(3.52f, 39.f)), module, Quale::CHORD_INPUT));
        addOutput(createOutput<AriaJackOut>(mm2px(Vec(3.52f, 59.f)), module, Quale::SCALE_OUTPUT));

        addInput(createInput<AriaJackIn>(mm2px(Vec(3.52f, 83.f)), module, Quale::SCALE_INPUT));
        addOutput(createOutput<AriaJackOut>(mm2px(Vec(3.52f, 103.f)), module, Quale::CHORD_OUTPUT));

        // Operation lights
        addChild(createLight<SmallLight<InputLight>>(mm2px(Vec(3.5f, 96.f)), module, Quale::SCALE_TO_CHORD_LIGHT));

        // Expander lights (right is 3.5mm from edge)
        addChild(createLight<SmallLight<InputLight>>(mm2px(Vec(1.4f, 125.2f)), module, Quale::EXPANDER_IN_LIGHT));
        addChild(createLight<SmallLight<OutputLight>>(mm2px(Vec(11.74f, 125.2f)), module, Quale::EXPANDER_OUT_LIGHT));
    }
};

} // namespace Quale

Model* modelQuale = createModel<Quale::Quale, Quale::QualeWidget>("Quale");
