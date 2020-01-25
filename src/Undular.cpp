#include "plugin.hpp"

struct Undular : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		U_INPUT,
		D_INPUT,
		L_INPUT,
		R_INPUT,
		X_INPUT,
		Y_INPUT,
		Z_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		DEBUG_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};
	
	dsp::ClockDivider scrollDivider;
	
	Undular() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		scrollDivider.setDivision(128);
	}
	
	void process(const ProcessArgs& args) override {
		if (scrollDivider.process()){
			math::Vec position = APP->scene->rackScroll->offset;
			position.y = 42400.f + (300.f * inputs[Y_INPUT].getVoltage());
			APP->scene->rackScroll->offset = position;
			outputs[DEBUG_OUTPUT].setVoltage(position.y);
			// Min: 42400
			// Max: 45400
		}
	}
};


struct UndularWidget : ModuleWidget {
	UndularWidget(Undular* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Undular.svg")));
		
		// Signature 
		addChild(createWidget<AriaSignature>(mm2px(Vec(1.0, 114.538))));

		// Screws
		addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		
		// Inputs
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 20.0)), module, Undular::U_INPUT));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 30.0)), module, Undular::D_INPUT));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 40.0)), module, Undular::L_INPUT));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 50.0)), module, Undular::R_INPUT));
		
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 70.0)), module, Undular::X_INPUT));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 80.0)), module, Undular::Y_INPUT));
		
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 100.0)), module, Undular::Z_INPUT));
		
		addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(7.62, 119.0)), module, Undular::DEBUG_OUTPUT));
	}
};

Model* modelUndular = createModel<Undular, UndularWidget>("Undular");