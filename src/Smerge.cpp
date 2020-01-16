#include "plugin.hpp"

struct Smerge : Module {
	enum ParamIds {
		SORT_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		LINK_INPUT,
		ENUMS(MERGE_INPUT, 16),
		NUM_INPUTS
	};
	enum OutputIds {
		POLY_OUTPUT,
		LINK_OUTPUT,
		DEBUG_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		LINK_IN_LIGHT,
		LINK_OUT_LIGHT,
		NUM_LIGHTS
	};
	
	dsp::ClockDivider ledDivider;

	Smerge() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		ledDivider.setDivision(4096);
		configParam(SORT_PARAM, 0.f, 1.f, 0.f, "Sort voltages");
	}
	
	// Merge without sorting, faster
	void merge(const ProcessArgs& args) {
		int lastMergeChannel = 0;
		for (int i = 0; i < 16; i++) {
			if (inputs[MERGE_INPUT + i].isConnected()) {
				outputs[POLY_OUTPUT].setVoltage(inputs[MERGE_INPUT + i].getVoltage(), i);
				lastMergeChannel = i+1;
			} else {
				outputs[POLY_OUTPUT].setVoltage(0, i);
			}
		}
		outputs[POLY_OUTPUT].setChannels(lastMergeChannel);
	}
	
	// Merge with sorting, and send Link output
	void mergeSortLink(const ProcessArgs& args) {
		std::array<std::array<float, 2>, 16> mergedVoltages;	
		int connected = 0;
		
		if (inputs[LINK_INPUT].isConnected()) {
			// Link input
			bool lastFound = false;
			for (int i = 15; i >= 0; i--) {
				mergedVoltages[i][0] = inputs[MERGE_INPUT + i].getVoltage();
				mergedVoltages[i][1] = inputs[LINK_INPUT].getVoltage(i);
				if (inputs[MERGE_INPUT + i].getVoltage() != 0.f)
					lastFound = true;
				if ((inputs[MERGE_INPUT + i].getVoltage() == 0.f) and (!lastFound))
					connected = i;
			}
			std::sort(mergedVoltages.begin(), mergedVoltages.begin() + connected, [](const std::array<float, 2> &left, const std::array<float, 2> &right) {
				if (left[1] == 0.f)
					return false;
				return left[1] < right[1];
			});	
		} else {
			// No link input
			for (int i = 0; i < 16; i++) {
				if (inputs[MERGE_INPUT + i].isConnected()) {
					mergedVoltages[i][0] = inputs[MERGE_INPUT + i].getVoltage();
					mergedVoltages[i][1] = (i + 1.f) * 0.1f;
					connected = i + 1;
				} else {
					mergedVoltages[i][0] = 0.f;
					mergedVoltages[i][1] = 0.f;
				}
			}
			std::sort(mergedVoltages.begin(), mergedVoltages.begin() + connected);		
		}
		
		// Send to poly output
		for (int i = 0; i < connected; i++) {
			outputs[POLY_OUTPUT].setVoltage(mergedVoltages[i][0], i);
		}
		outputs[POLY_OUTPUT].setChannels(connected);
		
		// Send to link output
		if (! inputs[LINK_INPUT].isConnected()) {
			outputs[LINK_OUTPUT].setChannels(connected);
			for (int i = 0; i < 16; i++) {
				outputs[LINK_OUTPUT].setVoltage(mergedVoltages[i][1], i);
			}
		}
	}
	
	void chainLink(const ProcessArgs& args) {
		if (inputs[LINK_INPUT].isConnected()) {
			outputs[LINK_OUTPUT].setChannels(inputs[LINK_INPUT].getChannels());
			for (int i = 0; i < 16; i++) {
				outputs[LINK_OUTPUT].setVoltage(inputs[LINK_INPUT].getVoltage(i), i);
			}
		} else {
			if (! params[SORT_PARAM].getValue()) {
				outputs[LINK_OUTPUT].setChannels(0);
			}
		}
	}
	
	
	void updateLeds(const ProcessArgs& args) {
		if ( (params[SORT_PARAM].getValue()) or (inputs[LINK_INPUT].isConnected()) ) {
			lights[LINK_IN_LIGHT].setBrightness(1.f);
			lights[LINK_OUT_LIGHT].setBrightness(1.f);
		} else {
			lights[LINK_IN_LIGHT].setBrightness(0.f);
			lights[LINK_OUT_LIGHT].setBrightness(0.f);
		}	
	}


	void process(const ProcessArgs& args) override {
		(params[SORT_PARAM].getValue()) ? mergeSortLink(args) : merge(args);
		chainLink(args); // Chain link inputs, whether sorting or not
		if (ledDivider.process())
			updateLeds(args);
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

		// Merge Output
		addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(12.7, 20.0)),  module, Smerge::POLY_OUTPUT));
		
		// Merge Inputs
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 29.5)),  module, Smerge::MERGE_INPUT + 0));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 37.5)),  module, Smerge::MERGE_INPUT + 1));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 45.5)),  module, Smerge::MERGE_INPUT + 2));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 53.5)),  module, Smerge::MERGE_INPUT + 3));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 61.5)),  module, Smerge::MERGE_INPUT + 4));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 69.5)),  module, Smerge::MERGE_INPUT + 5));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 77.5)),  module, Smerge::MERGE_INPUT + 6));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 85.5)),  module, Smerge::MERGE_INPUT + 7));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(17.78, 29.5)), module, Smerge::MERGE_INPUT + 8));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(17.78, 37.5)), module, Smerge::MERGE_INPUT + 9));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(17.78, 45.5)), module, Smerge::MERGE_INPUT + 10));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(17.78, 53.5)), module, Smerge::MERGE_INPUT + 11));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(17.78, 61.5)), module, Smerge::MERGE_INPUT + 12));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(17.78, 69.5)), module, Smerge::MERGE_INPUT + 13));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(17.78, 77.5)), module, Smerge::MERGE_INPUT + 14));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(17.78, 85.5)), module, Smerge::MERGE_INPUT + 15));

		// Sort button
		addParam(createParamCentered<AriaPushButton700>(mm2px(Vec(12.7, 95.0)), module, Smerge::SORT_PARAM));

		// Link jacks with lights
		addChild(createLightCentered<AriaJackLight<InputLight>>(mm2px(Vec(5.62, 109.0)), module, Smerge::LINK_IN_LIGHT));
		addChild(createLightCentered<AriaJackLight<OutputLight>>(mm2px(Vec(19.78, 109.0)), module, Smerge::LINK_OUT_LIGHT));
		
		addInput(createInputCentered<AriaJackTransparent>(mm2px(Vec(5.62, 109.0)), module, Smerge::LINK_INPUT));
		addOutput(createOutputCentered<AriaJackTransparent>(mm2px(Vec(19.78, 109.0)), module, Smerge::LINK_OUTPUT));
		
		// Debug Output
		#ifdef ARIA_DEBUG
		addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(7.62, 119.0)), module, Smerge::DEBUG_OUTPUT));
		#endif

	}
};


Model* modelSmerge = createModel<Smerge, SmergeWidget>("Smerge");