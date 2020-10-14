/*  Copyright (C) 2019-2020 Aria Salvatrice
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/
#include "plugin.hpp"

namespace Psychopump {

const int PROCESSDIVIDER = 32;

struct Psychopump : Module {
    enum ParamIds {
        ENUMS(CV1_PARAM, 8),
        ENUMS(CV2_PARAM, 8),
        ENUMS(CV3_PARAM, 8),
        ENUMS(CV4_PARAM, 8),
        ENUMS(CV5_PARAM, 8),
        ENUMS(CV6_PARAM, 8),
        ENUMS(CV7_PARAM, 8),
        ENUMS(CV8_PARAM, 8),
        NUM_PARAMS
    };
    enum InputIds {
        ENUMS(TRIGGER_INPUT, 8),
        ENUMS(CV_INPUT, 8),
        ENUMS(PITCH_INPUT, 8),
        NUM_INPUTS
    };
    enum OutputIds {
        CV1_OUTPUT,
        CV2_OUTPUT,
        CV3_OUTPUT,
        CV4_OUTPUT,
        CV5_OUTPUT,
        CV6_OUTPUT,
        CV7_OUTPUT,
        CV8_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        ROW1_LIGHT,
        ROW2_LIGHT,
        ROW3_LIGHT,
        ROW4_LIGHT,
        ROW5_LIGHT,
        ROW6_LIGHT,
        ROW7_LIGHT,
        ROW8_LIGHT,
        NUM_LIGHTS
    };

    dsp::ClockDivider processDivider;
    
    Psychopump() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        std::string label;
        for(size_t i = 0; i < 8; i++) {
            label = "Param 1 - Channel ";
            label.append(std::to_string(i + 1));
            configParam(CV1_PARAM + i, -10.f, 10.f, 0.f, label, "V");
            label = "Param 2 - Channel ";
            label.append(std::to_string(i + 1));
            configParam(CV2_PARAM + i, -10.f, 10.f, 0.f, label, "V");
            label = "Param 3 - Channel ";
            label.append(std::to_string(i + 1));
            configParam(CV3_PARAM + i, -10.f, 10.f, 0.f, label, "V");
            label = "Param 4 - Channel ";
            label.append(std::to_string(i + 1));
            configParam(CV4_PARAM + i, -10.f, 10.f, 0.f, label, "V");
            label = "Param 5 - Channel ";
            label.append(std::to_string(i + 1));
            configParam(CV5_PARAM + i, -10.f, 10.f, 0.f, label, "V");
            label = "Param 6 - Channel ";
            label.append(std::to_string(i + 1));
            configParam(CV6_PARAM + i, -10.f, 10.f, 0.f, label, "V");
            label = "Param 7 - Channel ";
            label.append(std::to_string(i + 1));
            configParam(CV7_PARAM + i, -10.f, 10.f, 0.f, label, "V");
            label = "Param 8 - Channel ";
            label.append(std::to_string(i + 1));
            configParam(CV8_PARAM + i, -10.f, 10.f, 0.f, label, "V");
        }
        processDivider.setDivision(PROCESSDIVIDER);
    }
    
    void process(const ProcessArgs& args) override {
        if (processDivider.process()) {

        }
    }
};


struct PsychopumpWidget : W::ModuleWidget {

void drawTriggerInputs(float xOffset, float yOffset, Psychopump* module) {
    for(size_t i = 0; i < 8; i++) {
        addStaticInput(mm2px(Vec(xOffset, yOffset + i * 10.f)), module, Psychopump::TRIGGER_INPUT + i);
    }
}

void drawCVParams(float xOffset, float yOffset, Psychopump* module) {
    for(size_t i = 0; i < 8; i++) {
        addChild(W::createKnobLight<W::KnobLightYellow>(mm2px(Vec(xOffset +  0.f, yOffset + i * 10.f)), module, Psychopump::ROW1_LIGHT, Psychopump::CV1_PARAM + i, -10.f, 10.f));
        addChild(W::createKnobLight<W::KnobLightYellow>(mm2px(Vec(xOffset + 10.f, yOffset + i * 10.f)), module, Psychopump::ROW2_LIGHT, Psychopump::CV2_PARAM + i, -10.f, 10.f));
        addChild(W::createKnobLight<W::KnobLightYellow>(mm2px(Vec(xOffset + 20.f, yOffset + i * 10.f)), module, Psychopump::ROW3_LIGHT, Psychopump::CV3_PARAM + i, -10.f, 10.f));
        addChild(W::createKnobLight<W::KnobLightYellow>(mm2px(Vec(xOffset + 30.f, yOffset + i * 10.f)), module, Psychopump::ROW4_LIGHT, Psychopump::CV4_PARAM + i, -10.f, 10.f));
        addChild(W::createKnobLight<W::KnobLightYellow>(mm2px(Vec(xOffset + 40.f, yOffset + i * 10.f)), module, Psychopump::ROW5_LIGHT, Psychopump::CV5_PARAM + i, -10.f, 10.f));
        addChild(W::createKnobLight<W::KnobLightYellow>(mm2px(Vec(xOffset + 50.f, yOffset + i * 10.f)), module, Psychopump::ROW6_LIGHT, Psychopump::CV6_PARAM + i, -10.f, 10.f));
        addChild(W::createKnobLight<W::KnobLightYellow>(mm2px(Vec(xOffset + 60.f, yOffset + i * 10.f)), module, Psychopump::ROW7_LIGHT, Psychopump::CV7_PARAM + i, -10.f, 10.f));
        addChild(W::createKnobLight<W::KnobLightYellow>(mm2px(Vec(xOffset + 70.f, yOffset + i * 10.f)), module, Psychopump::ROW8_LIGHT, Psychopump::CV8_PARAM + i, -10.f, 10.f));

        addParam(createParam<W::KnobTransparent>(mm2px(Vec(xOffset +  0.f, yOffset + i * 10.f)), module, Psychopump::CV1_PARAM + i));
        addParam(createParam<W::KnobTransparent>(mm2px(Vec(xOffset + 10.f, yOffset + i * 10.f)), module, Psychopump::CV2_PARAM + i));
        addParam(createParam<W::KnobTransparent>(mm2px(Vec(xOffset + 20.f, yOffset + i * 10.f)), module, Psychopump::CV3_PARAM + i));
        addParam(createParam<W::KnobTransparent>(mm2px(Vec(xOffset + 30.f, yOffset + i * 10.f)), module, Psychopump::CV4_PARAM + i));
        addParam(createParam<W::KnobTransparent>(mm2px(Vec(xOffset + 40.f, yOffset + i * 10.f)), module, Psychopump::CV5_PARAM + i));
        addParam(createParam<W::KnobTransparent>(mm2px(Vec(xOffset + 50.f, yOffset + i * 10.f)), module, Psychopump::CV6_PARAM + i));
        addParam(createParam<W::KnobTransparent>(mm2px(Vec(xOffset + 60.f, yOffset + i * 10.f)), module, Psychopump::CV7_PARAM + i));
        addParam(createParam<W::KnobTransparent>(mm2px(Vec(xOffset + 70.f, yOffset + i * 10.f)), module, Psychopump::CV8_PARAM + i));
    }
}

void drawCVOutputs(float xOffset, float yOffset, Psychopump* module) {
    addStaticOutput(mm2px(Vec(xOffset +  0.f, yOffset)), module, Psychopump::CV1_OUTPUT);
    addStaticOutput(mm2px(Vec(xOffset + 10.f, yOffset)), module, Psychopump::CV2_OUTPUT);
    addStaticOutput(mm2px(Vec(xOffset + 20.f, yOffset)), module, Psychopump::CV3_OUTPUT);
    addStaticOutput(mm2px(Vec(xOffset + 30.f, yOffset)), module, Psychopump::CV4_OUTPUT);
    addStaticOutput(mm2px(Vec(xOffset + 40.f, yOffset)), module, Psychopump::CV5_OUTPUT);
    addStaticOutput(mm2px(Vec(xOffset + 50.f, yOffset)), module, Psychopump::CV6_OUTPUT);
    addStaticOutput(mm2px(Vec(xOffset + 60.f, yOffset)), module, Psychopump::CV7_OUTPUT);
    addStaticOutput(mm2px(Vec(xOffset + 70.f, yOffset)), module, Psychopump::CV8_OUTPUT);
}

    PsychopumpWidget(Psychopump* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/faceplates/Psychopump.svg")));
        
        // Screws
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // TODO: Signature

        drawTriggerInputs(6.f, 30.f, module);
        drawCVParams(20.f, 30.f, module);
        drawCVOutputs(20.f, 115.f, module);
    }
};

} // namespace Psychopump

Model* modelPsychopump = createModel<Psychopump::Psychopump, Psychopump::PsychopumpWidget>("Psychopump");
