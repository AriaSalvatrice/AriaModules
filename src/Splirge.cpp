#include "plugin.hpp"


struct Splirge : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		POLYIN_INPUT,
		MERGE0_INPUT,
		MERGE1_INPUT,
		MERGE2_INPUT,
		MERGE3_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		SPLIT0_OUTPUT,
		SPLIT1_OUTPUT,
		SPLIT2_OUTPUT,
		SPLIT3_OUTPUT,
		POLYOUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Splirge() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void process(const ProcessArgs& args) override {
		// Haven't learned yet how to do this enum stuff that Fundamental 
		// uses in its split and merge code, so for now I'm gonna do it
		// the hard way rather than risk doing it the correct way.
		
		// Splitting
		outputs[SPLIT0_OUTPUT].setVoltage(inputs[POLYIN_INPUT].getVoltage(0));
		outputs[SPLIT1_OUTPUT].setVoltage(inputs[POLYIN_INPUT].getVoltage(1));
		outputs[SPLIT2_OUTPUT].setVoltage(inputs[POLYIN_INPUT].getVoltage(2));
		outputs[SPLIT3_OUTPUT].setVoltage(inputs[POLYIN_INPUT].getVoltage(3));
		
		// Merging
		int lastMergeChannel = 0;
		if (inputs[MERGE0_INPUT].isConnected()) {
			outputs[POLYOUT_OUTPUT].setVoltage(inputs[MERGE0_INPUT].getVoltage(), 0);
			lastMergeChannel = 1;
		}
		if (inputs[MERGE1_INPUT].isConnected()) {
			outputs[POLYOUT_OUTPUT].setVoltage(inputs[MERGE1_INPUT].getVoltage(), 1);
			lastMergeChannel = 2;
		}
		if (inputs[MERGE2_INPUT].isConnected()) {
			outputs[POLYOUT_OUTPUT].setVoltage(inputs[MERGE2_INPUT].getVoltage(), 2);
			lastMergeChannel = 3;
		}
		if (inputs[MERGE3_INPUT].isConnected()) {
			outputs[POLYOUT_OUTPUT].setVoltage(inputs[MERGE3_INPUT].getVoltage(), 3);
			lastMergeChannel = 4;
		}
		outputs[POLYOUT_OUTPUT].setChannels(lastMergeChannel);
	}
};


struct SplirgeWidget : ModuleWidget {
	SplirgeWidget(Splirge* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Splirge.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 20.5)), module, Splirge::POLYIN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 69.5)), module, Splirge::MERGE0_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 78.5)), module, Splirge::MERGE1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 87.5)), module, Splirge::MERGE2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 96.5)), module, Splirge::MERGE3_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 31.5)), module, Splirge::SPLIT0_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 40.5)), module, Splirge::SPLIT1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 49.5)), module, Splirge::SPLIT2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 58.5)), module, Splirge::SPLIT3_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 107.5)), module, Splirge::POLYOUT_OUTPUT));
	}
};


Model* modelSplirge = createModel<Splirge, SplirgeWidget>("Splirge");