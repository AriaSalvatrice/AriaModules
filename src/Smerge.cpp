#include "plugin.hpp"


struct Smerge : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		MERGE0_INPUT,
		MERGE8_INPUT,
		MERGE1_INPUT,
		MERGE9_INPUT,
		MERGE2_INPUT,
		MERGE10_INPUT,
		MERGE3_INPUT,
		MERGE11_INPUT,
		MERGE4_INPUT,
		MERGE12_INPUT,
		MERGE5_INPUT,
		MERGE13_INPUT,
		MERGE6_INPUT,
		MERGE14_INPUT,
		MERGE7_INPUT,
		MERGE15_INPUT,
		LINK_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		POLY_OUTPUT,
		LINK_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Smerge() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void process(const ProcessArgs& args) override {
	}
};


struct SmergeWidget : ModuleWidget {
	SmergeWidget(Smerge* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Smerge.svg")));
		
		// Signature 
		addChild(createWidget<AriaSignature>(mm2px(Vec(5.899, 114.538))));

		// Screws
		addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Input
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(5.62, 107.5)), module, Smerge::LINK_INPUT));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 33.5)), module, Smerge::MERGE0_INPUT));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 41.5)), module, Smerge::MERGE1_INPUT));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 49.5)), module, Smerge::MERGE2_INPUT));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 57.5)), module, Smerge::MERGE3_INPUT));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 65.5)), module, Smerge::MERGE4_INPUT));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 73.5)), module, Smerge::MERGE5_INPUT));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 81.5)), module, Smerge::MERGE6_INPUT));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 89.5)), module, Smerge::MERGE7_INPUT));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(17.78, 33.5)), module, Smerge::MERGE8_INPUT));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(17.78, 41.5)), module, Smerge::MERGE9_INPUT));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(17.78, 49.5)), module, Smerge::MERGE10_INPUT));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(17.78, 57.5)), module, Smerge::MERGE11_INPUT));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(17.78, 65.5)), module, Smerge::MERGE12_INPUT));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(17.78, 73.5)), module, Smerge::MERGE13_INPUT));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(17.78, 81.5)), module, Smerge::MERGE14_INPUT));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(17.78, 89.5)), module, Smerge::MERGE15_INPUT));

		// Output
		addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(12.7, 21.5)), module, Smerge::POLY_OUTPUT));
		addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(19.78, 107.5)), module, Smerge::LINK_OUTPUT));

	}
};


Model* modelSmerge = createModel<Smerge, SmergeWidget>("Smerge");