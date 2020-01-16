#include "plugin.hpp"

struct Spleet : Module {
	enum ParamIds {
		SORT_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		ENUMS(POLY_INPUT, 2),
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(SPLIT_OUTPUT, 8),
		DEBUG_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		POLY_LIGHT,
		ENUMS(SPLIT_LIGHT, 8),
		CHAIN_LIGHT,
		NUM_LIGHTS
	};
	
	dsp::ClockDivider ledDivider;
	bool chainMode;

	Spleet() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		ledDivider.setDivision(4096);
		configParam(SORT_PARAM, 0.f, 1.f, 0.f, "Sort voltages on both banks");
	}

	// Split without sorting, faster
	void split(const ProcessArgs& args) {
		for (int i = 0; i < 4; i++) // First bank
			outputs[SPLIT_OUTPUT + i].setVoltage(inputs[POLY_INPUT + 0].getVoltage(i));
		if (chainMode) { // Second bank depends on chain mode
			for (int i = 4; i < 8; i++) 
				outputs[SPLIT_OUTPUT + i].setVoltage(inputs[POLY_INPUT + 0].getVoltage(i));
		} else {
			for (int i = 4; i < 8; i++)
				outputs[SPLIT_OUTPUT + i].setVoltage(inputs[POLY_INPUT + 1].getVoltage(i - 4));
		}
	}
	
	// Split with sorting
	void splitSort(const ProcessArgs& args) {
		std::array<float, 8> splitVoltages;	// First bank, or both if chained
		int connected = 0; // First bank, or both if chained
		std::array<float, 4> splitVoltagesSecond;	
		int connectedSecond = 0;
		
		// How many connected inputs?
		connected = inputs[POLY_INPUT + 0].getChannels();
		connected = ( (!chainMode) and (connected > 4)) ? 4 : connected;
		connectedSecond = inputs[POLY_INPUT + 1].getChannels();
				
		// Fill arrays
		for (int i = 0; i < 8; i++)
			splitVoltages[i] = (i < connected) ? (inputs[POLY_INPUT + 0].getVoltage(i)) : 0.f;
		for (int i = 0; i < 4; i++)
			splitVoltagesSecond[i] = (i < connectedSecond) ? (inputs[POLY_INPUT + 1].getVoltage(i)) : 0.f;
					
		// Sort and output
		std::sort(splitVoltages.begin(), splitVoltages.begin() + connected);
		std::sort(splitVoltagesSecond.begin(), splitVoltagesSecond.begin() + connectedSecond);
		for (int i = 0; i < 4; i++)
			outputs[SPLIT_OUTPUT + i].setVoltage(splitVoltages[i]);	
		for (int i = 0; i < 4; i++)
			outputs[SPLIT_OUTPUT + i + 4].setVoltage( (chainMode) ? (splitVoltages[i + 4]) : (splitVoltagesSecond[i]) );
	}
	
	void updateLeds(const ProcessArgs& args) {
		lights[CHAIN_LIGHT].setBrightness( (chainMode) ? 1.f : 0.f);
		
		// Split outputs
		for (int i = 0; i < 4; i++)
			lights[SPLIT_LIGHT + i].setBrightness( (inputs[POLY_INPUT + 0].getChannels() > i) ? 1.f : 0.f);
		if (chainMode) {
			for (int i = 4; i < 8; i++)
				lights[SPLIT_LIGHT + i].setBrightness( (inputs[POLY_INPUT + 0].getChannels() > i) ? 1.f : 0.f);
		} else {
			for (int i = 0; i < 4; i++)
				lights[SPLIT_LIGHT + i + 4].setBrightness( (inputs[POLY_INPUT + 1].getChannels() > i) ? 1.f : 0.f);
		}
	}
	
	void process(const ProcessArgs& args) override {
		chainMode = (inputs[POLY_INPUT + 1].isConnected()) ? false : true;
		(params[SORT_PARAM].getValue()) ? splitSort(args) : split(args);
		if (ledDivider.process())
			updateLeds(args);
	}	
};


struct SpleetWidget : ModuleWidget {
	SpleetWidget(Spleet* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Spleet.svg")));
		
		// Signature 
		addChild(createWidget<AriaSignature>(mm2px(Vec(1.0, 114.538))));

		// Screws
		addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		
		// Jacks, top to bottom.
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 20.0)), module, Spleet::POLY_INPUT + 0));
		
		addChild(createLightCentered<AriaJackLight<OutputLight>>(mm2px(Vec(7.62, 30.0)), module, Spleet::SPLIT_LIGHT + 0));
		addChild(createLightCentered<AriaJackLight<OutputLight>>(mm2px(Vec(7.62, 38.0)), module, Spleet::SPLIT_LIGHT + 1));
		addChild(createLightCentered<AriaJackLight<OutputLight>>(mm2px(Vec(7.62, 46.0)), module, Spleet::SPLIT_LIGHT + 2));
		addChild(createLightCentered<AriaJackLight<OutputLight>>(mm2px(Vec(7.62, 54.0)), module, Spleet::SPLIT_LIGHT + 3));
		
		addOutput(createOutputCentered<AriaJackTransparent>(mm2px(Vec(7.62, 30.0)), module, Spleet::SPLIT_OUTPUT + 0));
		addOutput(createOutputCentered<AriaJackTransparent>(mm2px(Vec(7.62, 38.0)), module, Spleet::SPLIT_OUTPUT + 1));
		addOutput(createOutputCentered<AriaJackTransparent>(mm2px(Vec(7.62, 46.0)), module, Spleet::SPLIT_OUTPUT + 2));
		addOutput(createOutputCentered<AriaJackTransparent>(mm2px(Vec(7.62, 54.0)), module, Spleet::SPLIT_OUTPUT + 3));
		
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 67.0)), module, Spleet::POLY_INPUT + 1));
		
		addChild(createLightCentered<AriaJackLight<OutputLight>>(mm2px(Vec(7.62, 77.0)), module, Spleet::SPLIT_LIGHT + 4));
		addChild(createLightCentered<AriaJackLight<OutputLight>>(mm2px(Vec(7.62, 85.0)), module, Spleet::SPLIT_LIGHT + 5));
		addChild(createLightCentered<AriaJackLight<OutputLight>>(mm2px(Vec(7.62, 93.0)), module, Spleet::SPLIT_LIGHT + 6));
		addChild(createLightCentered<AriaJackLight<OutputLight>>(mm2px(Vec(7.62, 101.0)), module, Spleet::SPLIT_LIGHT + 7));
		
		addOutput(createOutputCentered<AriaJackTransparent>(mm2px(Vec(7.62, 77.0)), module, Spleet::SPLIT_OUTPUT + 4));
		addOutput(createOutputCentered<AriaJackTransparent>(mm2px(Vec(7.62, 85.0)), module, Spleet::SPLIT_OUTPUT + 5));
		addOutput(createOutputCentered<AriaJackTransparent>(mm2px(Vec(7.62, 93.0)), module, Spleet::SPLIT_OUTPUT + 6));
		addOutput(createOutputCentered<AriaJackTransparent>(mm2px(Vec(7.62, 101.0)), module, Spleet::SPLIT_OUTPUT + 7));
		
		// Pushbutton
		addParam(createParam<AriaPushButton500>(mm2px(Vec(1.0, 107)), module, Spleet::SORT_PARAM));
		
		// Chain light
		addChild(createLightCentered<SmallLight<InputLight>>(mm2px(Vec(13.7, 63.6)), module, Spleet::CHAIN_LIGHT));

		// Debug Output
		#ifdef ARIA_DEBUG
		addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(7.62, 119.0)), module, Spleet::DEBUG_OUTPUT));
		#endif
	}
};

Model* modelSpleet = createModel<Spleet, SpleetWidget>("Spleet");