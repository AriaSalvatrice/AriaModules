/*  Copyright (C) 2019-2020 Aria Salvatrice
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

// Chords to scale, scales to chords. Expander connections with Qqqq.

#include "plugin.hpp"
#include "quantizer.hpp"
#include "polyexternalscale.hpp"

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

    bool channel1root = true;
    size_t rootNote = 0;
    std::array<bool, 12> scale;
    dsp::ClockDivider processDivider;
    PolyExternalScale::PESExpanderMessage pesExpanderProducerMessage;
    PolyExternalScale::PESExpanderMessage pesExpanderConsumerMessage;
    
    Quale() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        processDivider.setDivision(PROCESSDIVIDER);
        leftExpander.producerMessage = &pesExpanderProducerMessage;
        leftExpander.consumerMessage = &pesExpanderConsumerMessage;
    }

    json_t* dataToJson() override {
        json_t* rootJ = json_object();
        json_t *channel1rootJ = json_boolean(channel1root);
        json_object_set_new(rootJ, "channel1root", channel1rootJ);
        return rootJ;
    }

    void dataFromJson(json_t* rootJ) override {
        json_t *channel1rootJ = json_object_get(rootJ, "channel1root");
        if (channel1rootJ) {
            channel1root = json_boolean_value(channel1rootJ);
        }
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
            PolyExternalScale::PESExpanderMessage *message = (PolyExternalScale::PESExpanderMessage*) leftExpander.consumerMessage;
            if (outputs[CHORD_OUTPUT].isConnected()) {
                for (size_t i = 0; i < 12; i++) {
                    if (message->scale[i]) outputs[CHORD_OUTPUT].setVoltage((i * 1.f/12.f) , j++);
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
            if (channel1root) {
                rootNote = Quantizer::quantizeToPositionInOctave(inputs[CHORD_INPUT].getVoltage(0), Quantizer::validNotesInScale(Quantizer::CHROMATIC));
            }
        }

        if ((rightExpander.module and rightExpander.module->model == modelQqqq)
        ||  (rightExpander.module and rightExpander.module->model == modelQuack)
        ||  (rightExpander.module and rightExpander.module->model == modelQ)) {
            // We have an expander
            lights[EXPANDER_OUT_LIGHT].setBrightness(1.f);
            PolyExternalScale::PESExpanderMessage *message = (PolyExternalScale::PESExpanderMessage*) rightExpander.module->leftExpander.producerMessage;			
            for (size_t i = 0; i < 12; i++) message->scale[i] = scale[i];
            if (channel1root) {
                message->hasRootNote = true;
                message->rootNote = Quantizer::quantizeToPositionInOctave(inputs[CHORD_INPUT].getVoltage(0), Quantizer::validNotesInScale(Quantizer::CHROMATIC));
            } else {
                message->hasRootNote = false;
            }
            rightExpander.module->leftExpander.messageFlipRequested = true;
        } else {
            // We have no expander
            lights[EXPANDER_OUT_LIGHT].setBrightness(0.f);
        }

        if (outputs[SCALE_OUTPUT].isConnected()){
            for (size_t i = 0; i < 12; i++) {
                if (channel1root && rootNote == i) {
                    outputs[SCALE_OUTPUT].setVoltage(10.f, i);    
                } else {
                    outputs[SCALE_OUTPUT].setVoltage( (scale[i]) ? 8.f : 0.f, i);
                }
            }
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


struct QualeSettingChannel1Root : MenuItem {
    Quale* module;
    size_t num;
    void onAction(const event::Action &e) override {
        module->channel1root = ! module->channel1root;
    }
};


struct QualeWidget : W::ModuleWidget {
    QualeWidget(Quale* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/faceplates/Quale.svg")));

        // Signature
        addChild(createWidget<W::Signature>(mm2px(Vec(1.0f, 114.5f))));
        
        // Screws
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Jacks
        addStaticInput(mm2px(Vec(3.52f, 39.f)), module, Quale::CHORD_INPUT);
        addStaticOutput(mm2px(Vec(3.52f, 59.f)), module, Quale::SCALE_OUTPUT);

        addStaticInput(mm2px(Vec(3.52f, 83.f)), module, Quale::SCALE_INPUT);
        addStaticOutput(mm2px(Vec(3.52f, 103.f)), module, Quale::CHORD_OUTPUT);

        // Operation lights
        addChild(createLight<W::StatusLightInput>(mm2px(Vec(3.5f, 96.f)), module, Quale::SCALE_TO_CHORD_LIGHT));

        // Expander lights (right is 3.5mm from edge)
        addChild(createLight<W::StatusLightInput>(mm2px(Vec(1.4f, 125.2f)), module, Quale::EXPANDER_IN_LIGHT));
        addChild(createLight<W::StatusLightOutput>(mm2px(Vec(11.74f, 125.2f)), module, Quale::EXPANDER_OUT_LIGHT));
    }

    void appendContextMenu(ui::Menu *menu) override {	
        Quale *module = dynamic_cast<Quale*>(this->module);
        assert(module);

        menu->addChild(new MenuSeparator());
        menu->addChild(createMenuLabel("Poly External Scales"));

        QualeSettingChannel1Root *qualeSettingChannel1Root = createMenuItem<QualeSettingChannel1Root>("Channel 1 of chord is P.E.S. root note", "");
        qualeSettingChannel1Root->module = module;
        qualeSettingChannel1Root->rightText += (module->channel1root) ? "✔" : "";
        menu->addChild(qualeSettingChannel1Root);
    }

};

} // namespace Quale

Model* modelQuale = createModel<Quale::Quale, Quale::QualeWidget>("Quale");
