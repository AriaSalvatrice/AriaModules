#include "plugin.hpp"


struct Splirge : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		POLY_INPUT,
		ENUMS(MERGE_INPUT, 4),
		NUM_INPUTS
	};
	enum OutputIds {
		POLY_OUTPUT,
		ENUMS(SPLIT_OUTPUT, 4),
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Splirge() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void process(const ProcessArgs& args) override {
		// Splitting
		for (int i = 0; i < 4; i++) {
			outputs[SPLIT_OUTPUT + i].setVoltage(inputs[POLY_INPUT].getVoltage(i));
		}
		
		// Merging
		int lastMergeChannel = 0;
		for (int i = 0; i < 4; i++) {
			if (inputs[MERGE_INPUT + i].isConnected()) {
				outputs[POLY_OUTPUT].setVoltage(inputs[MERGE_INPUT + i].getVoltage(), i);
				lastMergeChannel = i+1;
			}
		}
		outputs[POLY_OUTPUT].setChannels(lastMergeChannel);
	}
};


struct SplirgeWidget : ModuleWidget {
	SplirgeWidget(Splirge* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Splirge.svg")));
		
		// Signature 
		addChild(createWidget<AriaSignature>(mm2px(Vec(1.0, 114.538))));

		// Screws
		addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Inputs
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 23.5)), module, Splirge::POLY_INPUT));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 73.5)), module, Splirge::MERGE_INPUT + 0));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 81.5)), module, Splirge::MERGE_INPUT + 1));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 89.5)), module, Splirge::MERGE_INPUT + 2));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 97.5)), module, Splirge::MERGE_INPUT + 3));

		// Outputs
		addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(7.62, 107.5)), module, Splirge::POLY_OUTPUT));
		addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(7.62, 33.5)), module, Splirge::SPLIT_OUTPUT + 0));
		addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(7.62, 41.5)), module, Splirge::SPLIT_OUTPUT + 1));
		addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(7.62, 49.5)), module, Splirge::SPLIT_OUTPUT + 2));
		addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(7.62, 57.5)), module, Splirge::SPLIT_OUTPUT + 3));
	}
};


Model* modelSplirge = createModel<Splirge, SplirgeWidget>("Splirge");