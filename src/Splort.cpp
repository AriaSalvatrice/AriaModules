#include "plugin.hpp"


struct Splort : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		POLY_INPUT,
		LINK_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		SPLIT0_OUTPUT,
		SPLIT8_OUTPUT,
		SPLIT1_OUTPUT,
		SPLIT9_OUTPUT,
		SPLIT2_OUTPUT,
		SPLIT10_OUTPUT,
		SPLIT3_OUTPUT,
		SPLIT11_OUTPUT,
		SPLIT4_OUTPUT,
		SPLIT12_OUTPUT,
		SPLIT5_OUTPUT,
		SPLIT13_OUTPUT,
		SPLIT6_OUTPUT,
		SPLIT14_OUTPUT,
		SPLIT7_OUTPUT,
		SPLIT15_OUTPUT,
		LINK_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Splort() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void process(const ProcessArgs& args) override {
	}
};


struct SplortWidget : ModuleWidget {
	SplortWidget(Splort* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Splort.svg")));
		
		// Signature 
		addChild(createWidget<AriaSignature>(mm2px(Vec(5.899, 114.538))));

		// Screws
		addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Input
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(12.7, 21.5)), module, Splort::POLY_INPUT));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(5.62, 107.5)), module, Splort::LINK_INPUT));

		// Output
		addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(7.62, 33.5)), module, Splort::SPLIT0_OUTPUT));
		addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(7.62, 41.5)), module, Splort::SPLIT1_OUTPUT));
		addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(7.62, 49.5)), module, Splort::SPLIT2_OUTPUT));
		addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(7.62, 57.5)), module, Splort::SPLIT3_OUTPUT));
		addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(7.62, 65.5)), module, Splort::SPLIT4_OUTPUT));
		addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(7.62, 73.5)), module, Splort::SPLIT5_OUTPUT));
		addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(7.62, 81.5)), module, Splort::SPLIT6_OUTPUT));
		addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(7.62, 89.5)), module, Splort::SPLIT7_OUTPUT));
		addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(17.78, 33.5)), module, Splort::SPLIT8_OUTPUT));
		addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(17.78, 41.5)), module, Splort::SPLIT9_OUTPUT));
		addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(17.78, 49.5)), module, Splort::SPLIT10_OUTPUT));
		addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(17.78, 57.5)), module, Splort::SPLIT11_OUTPUT));
		addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(17.78, 65.5)), module, Splort::SPLIT12_OUTPUT));
		addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(17.78, 73.5)), module, Splort::SPLIT13_OUTPUT));
		addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(17.78, 81.5)), module, Splort::SPLIT14_OUTPUT));
		addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(17.78, 89.5)), module, Splort::SPLIT15_OUTPUT));
		addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(19.78, 107.5)), module, Splort::LINK_OUTPUT));
	}
};


Model* modelSplort = createModel<Splort, SplortWidget>("Splort");