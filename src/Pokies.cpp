/*  Copyright (C) 2019-2020 Aria Salvatrice
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
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

    bool globalTriggerStatus = false;
    float min[BUTTONS];
    float max[BUTTONS];
    std::array<bool, BUTTONS> enabled; // Only used in toggle mode
    std::array<bool, BUTTONS> momentary;
    dsp::ClockDivider processDivider;
    dsp::SchmittTrigger globalTrigger;
    dsp::SchmittTrigger toggleTrigger[BUTTONS];

    
    Pokies() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        for(size_t i = 0; i < BUTTONS; i++){
            configParam(POKIE_PARAM + i, 0.f, 1.f, 0.f, "Pokie " + std::to_string(i + 1));
            min[i] = 0.f;
            max[i] = 10.f;
            enabled[i] = false;
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

        json_t *enabledJ = json_array();
        for (size_t i = 0; i < BUTTONS; i++) json_array_insert_new(enabledJ, i, json_boolean(enabled[i]));
        json_object_set_new(rootJ, "enabled", enabledJ);

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

        json_t *enabledJ = json_object_get(rootJ, "enabled");
        if (enabledJ) {
            for (size_t i = 0; i < BUTTONS; i++) {
                json_t *enabledValueJ = json_array_get(enabledJ, i);
                if (enabledValueJ) enabled[i] = json_boolean_value(enabledValueJ);
            }
        }
    }

    
    void process(const ProcessArgs& args) override {
        if (processDivider.process()) {

            globalTriggerStatus = (globalTrigger.process(inputs[GLOBAL_INPUT].getVoltageSum())) ? true : false;

            for (size_t i = 0; i < BUTTONS; i++) {
                float voltage = min[i];

                if (momentary[i]) {
                    if (params[POKIE_PARAM + i].getValue() == 1.f) voltage = max[i];
                    if (globalTriggerStatus == true) voltage = max[i];
                } else {
                    if (globalTriggerStatus == true) {
                        enabled[i] = ! enabled[i]; 
                    } else {
                        if (toggleTrigger[i].process(params[POKIE_PARAM + i].getValue())) enabled[i] = ! enabled[i]; 
                    }
                    if (enabled[i]) voltage = max[i];
                }
                outputs[CV_OUTPUT + i].setVoltage(voltage);
            }

        }
    }

};




///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////




// Add a margin to my normal button, so the square that shows it's bound to MIDI is offset a bit.
// A black placeholder square is added to the faceplate. Positioning of the rectangle is yolo'd.
// The button is always momentary: we display an overlay when it's in toggle mode.
struct Pokie : W::ButtonMomentary {
    Pokie() {
        W::ButtonMomentary();
        box.size.x += mm2px(1.35f);
        box.size.y += mm2px(0.71f);
    }
};

// Light overlay
template <typename TModule>
struct PokieLight : TransparentWidget {
	TModule* module;
    size_t num;
	FramebufferWidget* framebuffer;
	SvgWidget* svgWidget;
    bool lastStatus;

    PokieLight() {
        framebuffer = new FramebufferWidget;
        addChild(framebuffer);
        svgWidget = new SvgWidget;
        svgWidget->setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-light-only.svg")));
        framebuffer->box.size = svgWidget->box.size;
        box.size = svgWidget->box.size;
        framebuffer->addChild(svgWidget);
        lastStatus = true;
    }

    void step() override {
        if(module) {
            if (module->enabled[num] != lastStatus) {
                framebuffer->visible = (module->enabled[num] == true && module->momentary[num] == false) ? true : false;
            }
            lastStatus = (module->enabled[num] == true && module->momentary[num] == false);
        }
        Widget::step();
    }
};



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
struct PokieSettingsItem : MenuItem {
    Pokies<KNOBS>* module;
    size_t num;

    struct MinMaxSliderItem : ui::Slider {
        MinMaxSliderItem(float *voltage, std::string label) {
            quantity = new MinMaxQuantity(voltage, label);
        }
        ~MinMaxSliderItem() {
            delete quantity;
        }
    };

    struct PokieSettingMomentary : MenuItem {
        Pokies<KNOBS>* module;
        size_t num;
        void onAction(const event::Action &e) override {
            module->momentary[num] = true;
        }
    };

    struct PokieSettingToggle : MenuItem {
        Pokies<KNOBS>* module;
        size_t num;
        void onAction(const event::Action &e) override {
            module->momentary[num] = false;
        }
    };

    struct PokieSettingUnipolar : MenuItem {
        Pokies<KNOBS>* module;
        size_t num;
        void onAction(const event::Action &e) override {
            module->min[num] = 0.f;
            module->max[num] = 10.f;
        }
    };

    struct PokieSettingUnipolar5v : MenuItem {
        Pokies<KNOBS>* module;
        size_t num;
        void onAction(const event::Action &e) override {
            module->min[num] = 0.f;
            module->max[num] = 5.f;
        }
    };

    struct PokieSettingBipolar : MenuItem {
        Pokies<KNOBS>* module;
        size_t num;
        void onAction(const event::Action &e) override {
            module->min[num] = -5.f;
            module->max[num] = 5.f;
        }
    };

    struct PokieSettingUnipolarInverted : MenuItem {
        Pokies<KNOBS>* module;
        size_t num;
        void onAction(const event::Action &e) override {
            module->min[num] = 10.f;
            module->max[num] = 0.f;
        }
    };

    struct PokieSettingUnipolar5vInverted : MenuItem {
        Pokies<KNOBS>* module;
        size_t num;
        void onAction(const event::Action &e) override {
            module->min[num] = 5.f;
            module->max[num] = 0.f;
        }
    };

    struct PokieSettingBipolarInverted : MenuItem {
        Pokies<KNOBS>* module;
        size_t num;
        void onAction(const event::Action &e) override {
            module->min[num] = 5.f;
            module->max[num] = -5.f;
        }
    };

    Menu *createChildMenu() override {
        Menu *menu = new Menu;

        menu->addChild(createMenuLabel("Pokie " + std::to_string(num + 1)));

        PokieSettingMomentary *pokieSettingMomentary = createMenuItem<PokieSettingMomentary>("Momentary", "");
        pokieSettingMomentary->module = module;
        pokieSettingMomentary->num = num;
        pokieSettingMomentary->rightText += (module->momentary[num]) ? "✔" : "";
        menu->addChild(pokieSettingMomentary);

        PokieSettingToggle *pokieSettingToggle = createMenuItem<PokieSettingToggle>("Toggle", "");
        pokieSettingToggle->module = module;
        pokieSettingToggle->num = num;
        pokieSettingToggle->rightText += (module->momentary[num]) ? "" : "✔";
        menu->addChild(pokieSettingToggle);

        menu->addChild(new MenuSeparator());

        menu->addChild(createMenuLabel("Range (can be inverted)"));

		MinMaxSliderItem *minSliderItem = new MinMaxSliderItem(&module->min[num], "Minimum");
		minSliderItem->box.size.x = 190.f;
		menu->addChild(minSliderItem);

		MinMaxSliderItem *maxSliderItem = new MinMaxSliderItem(&module->max[num], "Maximum");
		maxSliderItem->box.size.x = 190.f;
		menu->addChild(maxSliderItem);

        menu->addChild(new MenuSeparator());

        menu->addChild(createMenuLabel("Presets"));

        PokieSettingUnipolar *pokieSettingUnipolar = createMenuItem<PokieSettingUnipolar>("Set to 0 V ~ 10 V", "");
        pokieSettingUnipolar->module = module;
        pokieSettingUnipolar->num = num;
        menu->addChild(pokieSettingUnipolar);

        PokieSettingUnipolar5v *pokieSettingUnipolar5v = createMenuItem<PokieSettingUnipolar5v>("Set to 0 V ~ 5 V", "");
        pokieSettingUnipolar5v->module = module;
        pokieSettingUnipolar5v->num = num;
        menu->addChild(pokieSettingUnipolar5v);

        PokieSettingBipolar *pokieSettingBipolar = createMenuItem<PokieSettingBipolar>("Set to -5 V ~ 5 V", "");
        pokieSettingBipolar->module = module;
        pokieSettingBipolar->num = num;
        menu->addChild(pokieSettingBipolar);

        menu->addChild(createMenuLabel("Inverted Presets"));

        PokieSettingUnipolarInverted *pokieSettingUnipolarInverted = createMenuItem<PokieSettingUnipolarInverted>("Set to 10 V ~ 0 V", "");
        pokieSettingUnipolarInverted->module = module;
        pokieSettingUnipolarInverted->num = num;
        menu->addChild(pokieSettingUnipolarInverted);

        PokieSettingUnipolar5vInverted *pokieSettingUnipolar5vInverted = createMenuItem<PokieSettingUnipolar5vInverted>("Set to 5 V ~ 0 V", "");
        pokieSettingUnipolar5vInverted->module = module;
        pokieSettingUnipolar5vInverted->num = num;
        menu->addChild(pokieSettingUnipolar5vInverted);

        PokieSettingBipolarInverted *pokieSettingBipolarInverted = createMenuItem<PokieSettingBipolarInverted>("Set to 5 V ~ -5 V", "");
        pokieSettingBipolarInverted->module = module;
        pokieSettingBipolarInverted->num = num;
        menu->addChild(pokieSettingBipolarInverted);

        return menu;
    }

};




///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////




struct PokiesWidget : W::ModuleWidget {

    void drawPokie(Pokies<4>* module, float y, int num) {
        addParam(createParam<Pokie>(mm2px(Vec(3.52f, y)), module, Pokies<4>::POKIE_PARAM + num));

        PokieLight<Pokies<4>>* pokieLight = new PokieLight<Pokies<4>>;
        pokieLight->box.pos = mm2px(Vec(3.52f, y));
        pokieLight->module = module;
        pokieLight->num = num;
        addChild(pokieLight);

        addStaticOutput(mm2px(Vec(3.52f, y + 10.f)), module, Pokies<4>::CV_OUTPUT + num);
    }

    PokiesWidget(Pokies<4>* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/faceplates/Pokies.svg")));
        
        // Signature
        addChild(createWidget<W::Signature>(mm2px(Vec(1.0f, 114.5f))));

        // Global Input
        addStaticInput(mm2px(Vec(3.52f, 15.9f)), module, Pokies<4>::GLOBAL_INPUT);

        // Pokies
        drawPokie(module, 31.f, 0);
        drawPokie(module, 52.f, 1);
        drawPokie(module, 73.f, 2);
        drawPokie(module, 94.f, 3);

        // Screws
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    }

    void appendContextMenu(ui::Menu *menu) override {	
        Pokies<4> *module = dynamic_cast<Pokies<4>*>(this->module);
        assert(module);

        menu->addChild(new MenuSeparator());

        for(size_t i = 0; i < 4; i++) {
            PokieSettingsItem<4> *pokieSettingsItem = createMenuItem<PokieSettingsItem<4>>("Pokie " + std::to_string(i + 1), RIGHT_ARROW);
            pokieSettingsItem->module = module;
            pokieSettingsItem->num = i;
            menu->addChild(pokieSettingsItem);
        }

    }

};

} // namespace Pokies

Model* modelPokies4 = createModel<Pokies::Pokies<4>, Pokies::PokiesWidget>("Pokies4");
