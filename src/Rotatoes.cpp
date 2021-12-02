/*  Copyright (C) 2019-2020 Aria Salvatrice
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

// Contains the Rotatoes module, and the Grabby module.
// A grabby is really just a rotato that's linear when you think about it.

#include "plugin.hpp"
#include "quantizer.hpp"

namespace Rotatoes {

// Nope, no audio rate option provided until someone makes a strong argument why they need it.
const int PROCESSDIVIDER = 32;

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
        ENUMS(QUANTIZE_LIGHT, KNOBS),
        NUM_LIGHTS
    };


    float min[KNOBS];
    float max[KNOBS];
    std::array<bool, KNOBS> quantize;
    std::array<bool, 12> scale;
    dsp::ClockDivider processDivider;
    

    Rotatoes() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        for(size_t i = 0; i < KNOBS; i++){
            if (KNOBS == 1) {
                // If there's only one, it's a grabby, not a rotato.
                configParam(ROTATO_PARAM, 0.f, 1.f, 0.f, "Grabby");
            } else {
                configParam(ROTATO_PARAM + i, 0.f, 1.f, 0.f, "Rotato " + std::to_string(i + 1));
            }
            min[i] = 0.f;
            max[i] = 10.f;
            quantize[i] = true; // True = Auto if Poly External Scale present.
        }

        processDivider.setDivision(PROCESSDIVIDER);
    }


    json_t* dataToJson() override {
        json_t* rootJ = json_object();

        json_t *minJ = json_array();
        for (size_t i = 0; i < KNOBS; i++) json_array_insert_new(minJ, i, json_real(min[i]));
        json_object_set_new(rootJ, "min", minJ);
        
        json_t *maxJ = json_array();
        for (size_t i = 0; i < KNOBS; i++) json_array_insert_new(maxJ, i, json_real(max[i]));
        json_object_set_new(rootJ, "max", maxJ);

        json_t *quantizeJ = json_array();
        for (size_t i = 0; i < KNOBS; i++) json_array_insert_new(quantizeJ, i, json_boolean(quantize[i]));
        json_object_set_new(rootJ, "quantize", quantizeJ);

        return rootJ;
    }


    void dataFromJson(json_t* rootJ) override {
        json_t *minJ = json_object_get(rootJ, "min");
        if (minJ) {
            for (size_t i = 0; i < KNOBS; i++) {
                json_t *minValueJ = json_array_get(minJ, i);
                if (minValueJ) min[i] = json_real_value(minValueJ);
            }
        }

        json_t *maxJ = json_object_get(rootJ, "max");
        if (maxJ) {
            for (size_t i = 0; i < KNOBS; i++) {
                json_t *maxValueJ = json_array_get(maxJ, i);
                if (maxValueJ) max[i] = json_real_value(maxValueJ);
            }
        }

        json_t *quantizeJ = json_object_get(rootJ, "quantize");
        if (quantizeJ) {
            for (size_t i = 0; i < KNOBS; i++) {
                json_t *quantizeValueJ = json_array_get(quantizeJ, i);
                if (quantizeValueJ) quantize[i] = json_boolean_value(quantizeValueJ);
            }
        }
    }


    void process(const ProcessArgs& args) override {
        if (processDivider.process()) {

            if (inputs[EXT_SCALE_INPUT].isConnected()) {
                for (size_t i = 0; i < 12; i++){
                    scale[i] = (inputs[EXT_SCALE_INPUT].getVoltage(i) > 0.1f) ? true : false;
                }
                for(size_t i = 0; i < KNOBS; i++) {
                    if (quantize[i]) {
                        outputs[CV_OUTPUT + i].setVoltage( Quantizer::quantize( rescale(params[ROTATO_PARAM + i].getValue(), 0.f, 1.f, min[i], max[i]), scale) );
                        lights[QUANTIZE_LIGHT + i].setBrightness(1.f);
                    } else {
                        outputs[CV_OUTPUT + i].setVoltage( rescale(params[ROTATO_PARAM + i].getValue(), 0.f, 1.f, min[i], max[i]) );
                        lights[QUANTIZE_LIGHT + i].setBrightness(0.f);
                    }
                }
            } else {
                for(size_t i = 0; i < KNOBS; i++) {
                    outputs[CV_OUTPUT + i].setVoltage( rescale(params[ROTATO_PARAM + i].getValue(), 0.f, 1.f, min[i], max[i]) );
                    lights[QUANTIZE_LIGHT + i].setBrightness( (quantize[i]) ? 0.25f : 0.f);
                }
            }

        }
    }
    
};




///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////




// Add a margin to my normal knob, so the square that shows it's bound to MIDI is offset a bit.
// A black placeholder square is added to the faceplate. Positioning of the rectangle is yolo'd.
struct KnobRotato : W::Knob {
    KnobRotato() {
        W::Knob();
        box.size.x += mm2px(1.35f);
        box.size.y += mm2px(0.71f);
    }
};

struct GrabbySlider : SvgSlider {
    GrabbySlider() {
        SvgSlider();
        setBackgroundSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/grabby-bg.svg")));
        setHandleSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/grabby-knob.svg")));
        maxHandlePos = mm2px(Vec(0.f, 0.4f));
        minHandlePos = mm2px(Vec(0.f, 62.f));
        box.size.x = mm2px(10.45f);
        box.size.y = mm2px(71.9f);
    }
};

// Code mostly copied from https://github.com/gluethegiant/gtg-rack/blob/master/src/gtgComponents.hpp
struct MinMaxQuantity : Quantity {
	float *voltage = NULL;
	std::string label = "";

	MinMaxQuantity(float *_voltage, std::string _label) {
		voltage = _voltage;
		label = _label;
	}
	void setValue(float value) override {
		*voltage = math::clamp(value, getMinValue(), getMaxValue());
	}
	float getValue() override {
		return *voltage;
	}
	float getMinValue() override { return -10.0f; }
	float getMaxValue() override { return 10.0f; }
	std::string getLabel() override { return label; }
	std::string getUnit() override { return " V"; }
};

template <size_t KNOBS>
struct RotatoSettingsItem : MenuItem {
    Rotatoes<KNOBS>* module;
    size_t knob;

    struct MinMaxSliderItem : ui::Slider {
        MinMaxSliderItem(float *voltage, std::string label) {
            quantity = new MinMaxQuantity(voltage, label);
        }
        ~MinMaxSliderItem() {
            delete quantity;
        }
    };

    struct RotatoSettingQuantizeAuto : MenuItem {
        Rotatoes<KNOBS>* module;
        size_t knob;
        void onAction(const event::Action &e) override {
            module->quantize[knob] = true;
        }
    };

    struct RotatoSettingQuantizeDisabled : MenuItem {
        Rotatoes<KNOBS>* module;
        size_t knob;
        void onAction(const event::Action &e) override {
            module->quantize[knob] = false;
        }
    };

    struct RotatoSettingUnipolar : MenuItem {
        Rotatoes<KNOBS>* module;
        size_t knob;
        void onAction(const event::Action &e) override {
            module->min[knob] = 0.f;
            module->max[knob] = 10.f;
        }
    };

    struct RotatoSettingUnipolar5v : MenuItem {
        Rotatoes<KNOBS>* module;
        size_t knob;
        void onAction(const event::Action &e) override {
            module->min[knob] = 0.f;
            module->max[knob] = 5.f;
        }
    };

    struct RotatoSettingBipolar : MenuItem {
        Rotatoes<KNOBS>* module;
        size_t knob;
        void onAction(const event::Action &e) override {
            module->min[knob] = -5.f;
            module->max[knob] = 5.f;
        }
    };

    struct RotatoSettingUnipolarInverted : MenuItem {
        Rotatoes<KNOBS>* module;
        size_t knob;
        void onAction(const event::Action &e) override {
            module->min[knob] = 10.f;
            module->max[knob] = 0.f;
        }
    };

    struct RotatoSettingUnipolar5vInverted : MenuItem {
        Rotatoes<KNOBS>* module;
        size_t knob;
        void onAction(const event::Action &e) override {
            module->min[knob] = 5.f;
            module->max[knob] = 0.f;
        }
    };

    struct RotatoSettingBipolarInverted : MenuItem {
        Rotatoes<KNOBS>* module;
        size_t knob;
        void onAction(const event::Action &e) override {
            module->min[knob] = 5.f;
            module->max[knob] = -5.f;
        }
    };

    struct RotatoSettingVoctC2C4 : MenuItem {
        Rotatoes<KNOBS>* module;
        size_t knob;
        void onAction(const event::Action &e) override {
            module->min[knob] = -2.f;
            module->max[knob] = 0.f;
        }
    };

    struct RotatoSettingVoctC4C6 : MenuItem {
        Rotatoes<KNOBS>* module;
        size_t knob;
        void onAction(const event::Action &e) override {
            module->min[knob] = 0.f;
            module->max[knob] = 2.f;
        }
    };

    Menu *createChildMenu() override {
        Menu *menu = new Menu;

        menu->addChild(createMenuLabel("Rotato " + std::to_string(knob + 1)));
        menu->addChild(new MenuSeparator());

        menu->addChild(createMenuLabel("Quantize to Poly External Scale"));

        RotatoSettingQuantizeAuto *rotatoSettingQuantizeAuto = createMenuItem<RotatoSettingQuantizeAuto>("Automatic", "");
        rotatoSettingQuantizeAuto->module = module;
        rotatoSettingQuantizeAuto->knob = knob;
        rotatoSettingQuantizeAuto->rightText += (module->quantize[knob]) ? "✔" : "";
        menu->addChild(rotatoSettingQuantizeAuto);

        RotatoSettingQuantizeDisabled *rotatoSettingQuantizeDisabled = createMenuItem<RotatoSettingQuantizeDisabled>("Disabled", "");
        rotatoSettingQuantizeDisabled->module = module;
        rotatoSettingQuantizeDisabled->knob = knob;
        rotatoSettingQuantizeDisabled->rightText += (module->quantize[knob]) ? "" : "✔";
        menu->addChild(rotatoSettingQuantizeDisabled);

        menu->addChild(new MenuSeparator());

        menu->addChild(createMenuLabel("Range (can be inverted)"));

		MinMaxSliderItem *minSliderItem = new MinMaxSliderItem(&module->min[knob], "Minimum");
		minSliderItem->box.size.x = 190.f;
		menu->addChild(minSliderItem);

		MinMaxSliderItem *maxSliderItem = new MinMaxSliderItem(&module->max[knob], "Maximum");
		maxSliderItem->box.size.x = 190.f;
		menu->addChild(maxSliderItem);

        menu->addChild(new MenuSeparator());

        menu->addChild(createMenuLabel("Presets"));

        RotatoSettingUnipolar *rotatoSettingUnipolar = createMenuItem<RotatoSettingUnipolar>("Set to 0 V ~ 10 V", "");
        rotatoSettingUnipolar->module = module;
        rotatoSettingUnipolar->knob = knob;
        menu->addChild(rotatoSettingUnipolar);

        RotatoSettingUnipolar5v *rotatoSettingUnipolar5v = createMenuItem<RotatoSettingUnipolar5v>("Set to 0 V ~ 5 V", "");
        rotatoSettingUnipolar5v->module = module;
        rotatoSettingUnipolar5v->knob = knob;
        menu->addChild(rotatoSettingUnipolar5v);

        RotatoSettingBipolar *rotatoSettingBipolar = createMenuItem<RotatoSettingBipolar>("Set to -5 V ~ 5 V", "");
        rotatoSettingBipolar->module = module;
        rotatoSettingBipolar->knob = knob;
        menu->addChild(rotatoSettingBipolar);

        menu->addChild(createMenuLabel("Inverted Presets"));

        RotatoSettingUnipolarInverted *rotatoSettingUnipolarInverted = createMenuItem<RotatoSettingUnipolarInverted>("Set to 10 V ~ 0 V", "");
        rotatoSettingUnipolarInverted->module = module;
        rotatoSettingUnipolarInverted->knob = knob;
        menu->addChild(rotatoSettingUnipolarInverted);

        RotatoSettingUnipolar5vInverted *rotatoSettingUnipolar5vInverted = createMenuItem<RotatoSettingUnipolar5vInverted>("Set to 5 V ~ 0 V", "");
        rotatoSettingUnipolar5vInverted->module = module;
        rotatoSettingUnipolar5vInverted->knob = knob;
        menu->addChild(rotatoSettingUnipolar5vInverted);

        RotatoSettingBipolarInverted *rotatoSettingBipolarInverted = createMenuItem<RotatoSettingBipolarInverted>("Set to 5 V ~ -5 V", "");
        rotatoSettingBipolarInverted->module = module;
        rotatoSettingBipolarInverted->knob = knob;
        menu->addChild(rotatoSettingBipolarInverted);

        menu->addChild(createMenuLabel("V/Oct range Presets"));

        RotatoSettingVoctC2C4 *rotatoSettingVoctC2C4 = createMenuItem<RotatoSettingVoctC2C4>("Set to C2 ~ C4", "");
        rotatoSettingVoctC2C4->module = module;
        rotatoSettingVoctC2C4->knob = knob;
        menu->addChild(rotatoSettingVoctC2C4);

        RotatoSettingVoctC4C6 *rotatoSettingVoctC4C6 = createMenuItem<RotatoSettingVoctC4C6>("Set to C4 ~ C6", "");
        rotatoSettingVoctC4C6->module = module;
        rotatoSettingVoctC4C6->knob = knob;
        menu->addChild(rotatoSettingVoctC4C6);

        return menu;
    }

};




///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////




struct Rotatoes4Widget : W::ModuleWidget {

    void drawRotato(Rotatoes<4>* module, float y, int num) {
        addParam(createParam<KnobRotato>(mm2px(Vec(3.52f, y)), module, Rotatoes<4>::ROTATO_PARAM + num));
        addStaticOutput(mm2px(Vec(3.52f, y + 10.f)), module, Rotatoes<4>::CV_OUTPUT + num);
        addChild(createLight<W::StatusLightInput>(mm2px(Vec(2.25f, y + 6.9f)), module, Rotatoes<4>::QUANTIZE_LIGHT + num));
    }

    Rotatoes4Widget(Rotatoes<4>* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/faceplates/Rotatoes.svg")));

        // Signature
        addChild(createWidget<W::Signature>(mm2px(Vec(1.0f, 114.5f))));

        // External
        addStaticInput(mm2px(Vec(3.52f, 15.9f)), module, Rotatoes<4>::EXT_SCALE_INPUT);

        // Rotatoes
        drawRotato(module, 31.f, 0);
        drawRotato(module, 52.f, 1);
        drawRotato(module, 73.f, 2);
        drawRotato(module, 94.f, 3);

        // Screws
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    }

    void appendContextMenu(ui::Menu *menu) override {	
        Rotatoes<4> *module = dynamic_cast<Rotatoes<4>*>(this->module);
        assert(module);

        menu->addChild(new MenuSeparator());

        for(size_t i = 0; i < 4; i++) {
            RotatoSettingsItem<4> *rotatoSettingsItem = createMenuItem<RotatoSettingsItem<4>>("Rotato " + std::to_string(i + 1), RIGHT_ARROW);
            rotatoSettingsItem->module = module;
            rotatoSettingsItem->knob = i;
            menu->addChild(rotatoSettingsItem);
        }

    }

};




///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////




struct GrabbyWidget : W::ModuleWidget {

    GrabbyWidget(Rotatoes<1>* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/faceplates/Grabby.svg")));

        // Signature
        addChild(createWidget<W::Signature>(mm2px(Vec(1.0f, 114.5f))));

        // External
        addStaticInput(mm2px(Vec(3.52f, 15.9f)), module, Rotatoes<1>::EXT_SCALE_INPUT);

        // Grabby
        addParam(createParam<GrabbySlider>(mm2px(Vec(2.62f, 31.f)), module, Rotatoes<1>::ROTATO_PARAM + 0));
        addStaticOutput(mm2px(Vec(3.52f, 104.f)), module, Rotatoes<1>::CV_OUTPUT + 0);
        addChild(createLight<W::StatusLightInput>(mm2px(Vec(2.25f, 100.9f)), module, Rotatoes<1>::QUANTIZE_LIGHT + 0));

        // Screws
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    }

    void appendContextMenu(ui::Menu *menu) override {	
        Rotatoes<1> *module = dynamic_cast<Rotatoes<1>*>(this->module);
        assert(module);

        menu->addChild(new MenuSeparator());

        // NOTE: Entirely copy-pasted from above code for multiple rotatoes. 
        // Yeah it's nasty looking but oh well.

        menu->addChild(createMenuLabel("Range (can be inverted)"));

		RotatoSettingsItem<1>::MinMaxSliderItem *minSliderItem = new RotatoSettingsItem<1>::MinMaxSliderItem(&module->min[0], "Minimum");
		minSliderItem->box.size.x = 190.f;
		menu->addChild(minSliderItem);

		RotatoSettingsItem<1>::MinMaxSliderItem *maxSliderItem = new RotatoSettingsItem<1>::MinMaxSliderItem(&module->max[0], "Maximum");
		maxSliderItem->box.size.x = 190.f;
		menu->addChild(maxSliderItem);

        menu->addChild(new MenuSeparator());

        menu->addChild(createMenuLabel("Presets"));

        RotatoSettingsItem<1>::RotatoSettingUnipolar *rotatoSettingUnipolar = createMenuItem<RotatoSettingsItem<1>::RotatoSettingUnipolar>("Set to 0 V ~ 10 V", "");
        rotatoSettingUnipolar->module = module;
        rotatoSettingUnipolar->knob = 0;
        menu->addChild(rotatoSettingUnipolar);

        RotatoSettingsItem<1>::RotatoSettingUnipolar5v *rotatoSettingUnipolar5v = createMenuItem<RotatoSettingsItem<1>::RotatoSettingUnipolar5v>("Set to 0 V ~ 5 V", "");
        rotatoSettingUnipolar5v->module = module;
        rotatoSettingUnipolar5v->knob = 0;
        menu->addChild(rotatoSettingUnipolar5v);

        RotatoSettingsItem<1>::RotatoSettingBipolar *rotatoSettingBipolar = createMenuItem<RotatoSettingsItem<1>::RotatoSettingBipolar>("Set to -5 V ~ 5 V", "");
        rotatoSettingBipolar->module = module;
        rotatoSettingBipolar->knob = 0;
        menu->addChild(rotatoSettingBipolar);

        menu->addChild(createMenuLabel("Inverted Presets"));

        RotatoSettingsItem<1>::RotatoSettingUnipolarInverted *rotatoSettingUnipolarInverted = createMenuItem<RotatoSettingsItem<1>::RotatoSettingUnipolarInverted>("Set to 10 V ~ 0 V", "");
        rotatoSettingUnipolarInverted->module = module;
        rotatoSettingUnipolarInverted->knob = 0;
        menu->addChild(rotatoSettingUnipolarInverted);

        RotatoSettingsItem<1>::RotatoSettingUnipolar5vInverted *rotatoSettingUnipolar5vInverted = createMenuItem<RotatoSettingsItem<1>::RotatoSettingUnipolar5vInverted>("Set to 5 V ~ 0 V", "");
        rotatoSettingUnipolar5vInverted->module = module;
        rotatoSettingUnipolar5vInverted->knob = 0;
        menu->addChild(rotatoSettingUnipolar5vInverted);

        RotatoSettingsItem<1>::RotatoSettingBipolarInverted *rotatoSettingBipolarInverted = createMenuItem<RotatoSettingsItem<1>::RotatoSettingBipolarInverted>("Set to 5 V ~ -5 V", "");
        rotatoSettingBipolarInverted->module = module;
        rotatoSettingBipolarInverted->knob = 0;
        menu->addChild(rotatoSettingBipolarInverted);

        menu->addChild(createMenuLabel("V/Oct range Presets"));

        RotatoSettingsItem<1>::RotatoSettingVoctC2C4 *rotatoSettingVoctC2C4 = createMenuItem<RotatoSettingsItem<1>::RotatoSettingVoctC2C4>("Set to C2 ~ C4", "");
        rotatoSettingVoctC2C4->module = module;
        rotatoSettingVoctC2C4->knob = 0;
        menu->addChild(rotatoSettingVoctC2C4);

        RotatoSettingsItem<1>::RotatoSettingVoctC4C6 *rotatoSettingVoctC4C6 = createMenuItem<RotatoSettingsItem<1>::RotatoSettingVoctC4C6>("Set to C4 ~ C6", "");
        rotatoSettingVoctC4C6->module = module;
        rotatoSettingVoctC4C6->knob = 0;
        menu->addChild(rotatoSettingVoctC4C6);

    }
};


} // Namespace Rotatoes

Model* modelRotatoes4 = createModel<Rotatoes::Rotatoes<4>, Rotatoes::Rotatoes4Widget>("Rotatoes4");
Model* modelGrabby = createModel<Rotatoes::Rotatoes<1>, Rotatoes::GrabbyWidget>("Grabby");
