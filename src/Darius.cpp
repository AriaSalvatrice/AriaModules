#include "plugin.hpp"
#include <random>

const int STEP1START = 0;  //               00        
const int STEP2START = 1;  //             02  01            
const int STEP3START = 3;  //           05  04  03          
const int STEP4START = 6;  //         09  08  07  06        
const int STEP5START = 10; //       14  13  12  11  10      
const int STEP6START = 15; //     20  19  18  17  16  15    
const int STEP7START = 21; //   27  26  25  24  23  22  21  
const int STEP8START = 28; // 35  34  33  32  31  30  29  28
const int STEP9START = 36; // (Panel is rotated 90 degrees counter-clockwise compared to this diagram)

// PRNG
// Using xoroshiro128+ like VCV does - gives me a better distribution than mersenne twister.
// http://prng.di.unimi.it/
// https://community.vcvrack.com/t/controlling-the-random-seed/8005
//
// Running an automated test, I'm obtaining a distribution of results that look sane, 
// not skewed to either side, which was a problem with std's mersenne twister.
namespace prng {

	static inline uint64_t rotl(const uint64_t x, int k) {
		return (x << k) | (x >> (64 - k));
	}

	static uint64_t s[2];

	uint64_t next(void) {
		const uint64_t s0 = s[0];
		uint64_t s1 = s[1];
		const uint64_t result = s0 + s1;

		s1 ^= s0;
		s[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16); // a, b
		s[1] = rotl(s1, 37); // c

		return result;
	}
	
	void init(float seed1, float seed2){
		s[0] = seed1 * 52852712; // Keyboard smash
		s[1] = seed2 * 60348921;
		for (int i = 0; i < 10; i++) next(); // Warm up for better results
	}

	float uniform() {
		return (next() >> (64 - 24)) / std::pow(2.f, 24);
	}

}


struct Darius : Module {
	enum ParamIds {
		ENUMS(CV_PARAM, 36),
		ENUMS(ROUTE_PARAM, 36),
		STEP_PARAM,
		RUN_PARAM,
		RESET_PARAM,
		STEPCOUNT_PARAM,
		RANDCV_PARAM,
		RANDROUTE_PARAM, // 1.2.0 release
		RANGE_PARAM,
		SEED_MODE_PARAM, // 1.3.0 release
		NUM_PARAMS
	};
	enum InputIds {
		RUN_INPUT,
		RESET_INPUT,
		STEP_INPUT, // 1.2.0 release
		STEP_BACK_INPUT,
		STEP_UP_INPUT,
		STEP_DOWN_INPUT,
		SEED_INPUT, // 1.3.0 release
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(GATE_OUTPUT, 36),
		CV_OUTPUT, // 1.2.0 release
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(CV_LIGHT, 36),
		ENUMS(GATE_LIGHT, 36), // 1.2.0 release
		NUM_LIGHTS
	};
	
	bool running = true;
	bool steppedForward = false;
	bool steppedBack = false;
	bool forceUp = false;
	bool forceDown = false;
	bool lightsReset = false;
	bool shSeedNextFirst = false; // S & H the seed next 1st step
	int stepCount = 8;
	int step = 0;
	int node = 0;
	int lastNode = 0;
	int lastGate = 0;
	int pathTraveled[8] = { 0, -1, -1, -1, -1, -1, -1, -1}; // -1 = not gone there yet
	float randomSeed = 0.f;
	dsp::SchmittTrigger stepUpCvTrigger;
	dsp::SchmittTrigger stepDownCvTrigger;
	dsp::SchmittTrigger stepBackCvTrigger;
	dsp::SchmittTrigger stepForwardCvTrigger;
	dsp::SchmittTrigger stepForwardButtonTrigger;
	dsp::SchmittTrigger runCvTrigger;
	dsp::SchmittTrigger resetCvTrigger;
	dsp::SchmittTrigger resetButtonTrigger;
	dsp::SchmittTrigger randomizeCvTrigger;
	dsp::SchmittTrigger randomizeRouteTrigger;

	Darius() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(STEP_PARAM, 0.f, 1.f, 0.f, "Step");
		configParam(RUN_PARAM, 0.f, 1.f, 1.f, "Run");
		configParam(RESET_PARAM, 0.f, 1.f, 0.f, "Reset");
		configParam(STEPCOUNT_PARAM, 1.f, 8.f, 8.f, "Steps");
		configParam(RANDCV_PARAM, 0.f, 1.f, 0.f, "Randomize CV knobs");
		configParam(RANDROUTE_PARAM, 0.f, 1.f, 0.f, "Meta-randomize random route knobs");
		configParam(SEED_MODE_PARAM, 0.f, 1.f, 0.f, "New random seed on first or all nodes");
		configParam(RANGE_PARAM, 0.f, 1.f, 0.f, "Voltage output range");
		for (int i = 0; i < STEP9START; i++)
			configParam(CV_PARAM + i, 0.f, 10.f, 5.f, "CV");
		for (int i = 0; i < STEP8START; i++)
			configParam(ROUTE_PARAM + i, 0.f, 1.f, 0.5f, "Random route");
	}
	
	json_t* dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "step", json_integer(step));
		json_object_set_new(rootJ, "node", json_integer(node));
		json_object_set_new(rootJ, "lastNode", json_integer(lastNode));
		json_object_set_new(rootJ, "lastGate", json_integer(lastGate));
		json_t *pathTraveledJ = json_array();
		for (int i = 0; i < 8; i++) {
			json_array_insert_new(pathTraveledJ, i, json_integer(pathTraveled[i]));
		} 
		json_object_set_new(rootJ, "pathTraveled", pathTraveledJ);
		return rootJ;
	}
	
	void dataFromJson(json_t* rootJ) override {
		json_t* stepJ = json_object_get(rootJ, "step");
		if (stepJ){
			step = json_integer_value(stepJ);
		}
		json_t* nodeJ = json_object_get(rootJ, "node");
		if (nodeJ){
			node = json_integer_value(nodeJ);
		}
		json_t* lastNodeJ = json_object_get(rootJ, "lastNode");
		if (lastNodeJ){
			lastNode = json_integer_value(lastNodeJ);
		}
		json_t* lastGateJ = json_object_get(rootJ, "lastGate");
		if (lastGateJ){
			lastGate = json_integer_value(lastGateJ);
		}
		json_t *pathTraveledJ = json_object_get(rootJ, "pathTraveled");
		if (pathTraveledJ) {
			for (int i = 0; i < 8; i++) {
				json_t *pathTraveledNodeJ = json_array_get(pathTraveledJ, i);
				if (pathTraveledNodeJ) {
					pathTraveled[i] = json_integer_value(pathTraveledNodeJ);
				}
			}
		}
		lightsReset = true;
	}

	// Undo/Redo for Randomize CV Button. Thanks to David O'Rourke for the example implementation!
	// https://github.com/AriaSalvatrice/AriaVCVModules/issues/14
	struct RandomizeCvAction : history::ModuleAction {
		std::array<float, 36> oldValues;
		std::array<float, 36> newValues;

		RandomizeCvAction(int moduleId, std::array<float, 36> oldValues, std::array<float, 36> newValues) {
			name = "randomize Darius CV";
			this->moduleId = moduleId;
			this->oldValues = oldValues;
			this->newValues = newValues;
		}

		void undo() override {
			Darius *module = dynamic_cast<Darius*>(APP->engine->getModule(this->moduleId));
			if (module) {
				for (int i = 0; i < 36; i++) module->params[CV_PARAM + i].setValue(this->oldValues[i]);
			}
		}

		void redo() override {
			Darius *module = dynamic_cast<Darius*>(APP->engine->getModule(this->moduleId));
			if (module) {
				for (int i = 0; i < 36; i++) module->params[CV_PARAM + i].setValue(this->newValues[i]);
			}
		}
	};

	void randomizeCv(const ProcessArgs& args){
		std::array<float, 36> oldValues;
		std::array<float, 36> newValues;
		for (int i = 0; i < 36; i++) oldValues[i] = params[CV_PARAM + i].getValue();
		for (int i = 0; i < 36; i++) params[CV_PARAM + i].setValue(random::uniform() * 10.f);
		for (int i = 0; i < 36; i++) newValues[i] = params[CV_PARAM + i].getValue();
		APP->history->push(new RandomizeCvAction(this->id, oldValues, newValues));
	}
	
	// Undo/Redo for Randomize Route Button.
	struct RandomizeRouteAction : history::ModuleAction {
		std::array<float, 36> oldValues;
		std::array<float, 36> newValues;

		RandomizeRouteAction(int moduleId, std::array<float, 36> oldValues, std::array<float, 36> newValues) {
			name = "randomize Darius Route";
			this->moduleId = moduleId;
			this->oldValues = oldValues;
			this->newValues = newValues;
		}

		void undo() override {
			Darius *module = dynamic_cast<Darius*>(APP->engine->getModule(this->moduleId));
			if (module) {
				for (int i = 0; i < 36; i++) module->params[ROUTE_PARAM + i].setValue(this->oldValues[i]);
			}
		}

		void redo() override {
			Darius *module = dynamic_cast<Darius*>(APP->engine->getModule(this->moduleId));
			if (module) {
				for (int i = 0; i < 36; i++) module->params[ROUTE_PARAM + i].setValue(this->newValues[i]);
			}
		}
	};

	void randomizeRoute(const ProcessArgs& args){
		std::array<float, 36> oldValues;
		std::array<float, 36> newValues;
		for (int i = 0; i < 36; i++) oldValues[i] = params[ROUTE_PARAM + i].getValue();
		for (int i = 0; i < 36; i++) params[ROUTE_PARAM + i].setValue(random::uniform());	
		for (int i = 0; i < 36; i++) newValues[i] = params[ROUTE_PARAM + i].getValue();
		APP->history->push(new RandomizeRouteAction(this->id, oldValues, newValues));
	}
	
	void resetPathTraveled(const ProcessArgs& args){
		pathTraveled[0] = 0;
		for (int i = 1; i < 8; i++) pathTraveled[i] = -1;
	}
	
	void refreshSeed(const ProcessArgs& args){
		if (inputs[SEED_INPUT].isConnected() and (inputs[SEED_INPUT].getVoltage() != 0.f) ) {
			randomSeed = inputs[SEED_INPUT].getVoltage();
		} else {
			randomSeed = random::uniform();
		}
	}
	
	void processReset(const ProcessArgs& args){
		if (resetCvTrigger.process(inputs[RESET_INPUT].getVoltageSum()) or resetButtonTrigger.process(params[RESET_PARAM].getValue())){
			step = 0;
			node = 0;
			lastNode = 0;
			lightsReset = true;
			shSeedNextFirst = true;
			resetPathTraveled(args);
			for (int i = 0; i < 36; i++)
				outputs[GATE_OUTPUT + i].setVoltage(0.f);
		}
	}
	
	void processRunStatus(const ProcessArgs& args){
		if (runCvTrigger.process(inputs[RUN_INPUT].getVoltageSum())){
			running = !running;
			params[RUN_PARAM].setValue(running);
		}
		running = params[RUN_PARAM].getValue();
	}
		
	void processStepNumber(const ProcessArgs& args){
		stepCount = std::round(params[STEPCOUNT_PARAM].getValue());
		if (running) {
			bool triggerAccepted = false; // Accept only one trigger!
			if (stepForwardCvTrigger.process(inputs[STEP_INPUT].getVoltageSum())){
				step++;
				steppedForward = true;
				triggerAccepted = true;
			}
			if (stepUpCvTrigger.process(inputs[STEP_UP_INPUT].getVoltageSum()) and !triggerAccepted){
				step++;
				forceUp = true;
				steppedForward = true;
				triggerAccepted = true;
			}
			if (stepDownCvTrigger.process(inputs[STEP_DOWN_INPUT].getVoltageSum()) and !triggerAccepted){
				step++;
				forceDown = true;
				steppedForward = true;
				triggerAccepted = true;
			}
			if (stepBackCvTrigger.process(inputs[STEP_BACK_INPUT].getVoltageSum()) and step > 0 and !triggerAccepted){
				step--;
				steppedBack = true;
			}
		}
		if (stepForwardButtonTrigger.process(params[STEP_PARAM].getValue())){
			step++; // You can still advance manually if module isn't running
			steppedForward = true;
		}
		lastGate = node;
		if (step >= stepCount) {
			shSeedNextFirst = true;
			step = 0;
			node = 0;
			lastNode = 0;
			resetPathTraveled(args);
			lightsReset = true;
		}
	}
	
	void processNodeForward(const ProcessArgs& args){
		steppedForward = false;
		
		// Refresh seed as lazily as possible
		if (step == 1 and shSeedNextFirst){
			refreshSeed(args);
			shSeedNextFirst = false;
		} else {
			if (params[SEED_MODE_PARAM].getValue() == 1.0f) refreshSeed(args);
		}
		
		if (step == 0){ // Step 1 starting
			node = 0;
			lightsReset = true;
		} else { // Step 2~8 starting
			if (forceUp or forceDown) {
				if (forceUp) {
					if (step == 1) {
						node = 1; // FIXME: This check prevents issue #21 but I don't understand why
					} else {
						node = node + step;
					}
					forceUp = false;
				}
				if (forceDown) {
					if (step == 1) {
						node = 2;
					} else {
						node = node + step + 1;
					}
					forceDown = false;
				}
			} else {
				prng::init(randomSeed, step);
				if (prng::uniform() < params[ROUTE_PARAM + lastNode].getValue()) {
					node = node + step + 1;
				} else {
					node = node + step;
				}
				
			}
		}
		pathTraveled[step] = node;
		lastNode = node;
	}
	
	void processNodeBack(const ProcessArgs& args){
		lightsReset = true;
		node = pathTraveled[step];
		// FIXME - This conditional avoids a bizarre problem where randomSeed goes NaN. Not sure what's exactly going on!!
		if (step < 7) pathTraveled[step + 1] = -1; 
		lastNode = node;
	}
	
	void processGateOutput(const ProcessArgs& args){
		if (inputs[STEP_INPUT].isConnected()){
			outputs[GATE_OUTPUT + node].setVoltage(inputs[STEP_INPUT].getVoltage());
		} else {
			outputs[GATE_OUTPUT + lastGate].setVoltage(0.f);
			outputs[GATE_OUTPUT + node].setVoltage(10.f);
		}
	}
	
	void processVoltageOutput(const ProcessArgs& args){
		if (params[RANGE_PARAM].getValue() == 0.f ) {
			outputs[CV_OUTPUT].setVoltage(params[CV_PARAM + node].getValue());
		} else {
			outputs[CV_OUTPUT].setVoltage(params[CV_PARAM + node].getValue() - 5.0);
		}
	}
	
	void processLights(const ProcessArgs& args){
		// Clean up by request only
		if (lightsReset) {
			for (int i = 0; i < 36; i++) lights[CV_LIGHT + i].setBrightness( 0.f );
			for (int i = 0; i < 8; i++) {
				if (pathTraveled[i] >= 0) lights[CV_LIGHT + pathTraveled[i]].setBrightness( 1.f );
			}
			lightsReset = false;
		}
		lights[CV_LIGHT + pathTraveled[step]].setBrightness( 1.f );
		// Light the outputs depending on amount of steps enabled
		lights[GATE_LIGHT].setBrightness( (stepCount >= 1 ) ? 1.f : 0.f );
		for (int i = STEP2START; i < STEP3START; i++)
			lights[GATE_LIGHT + i].setBrightness( (stepCount >= 2 ) ? 1.f : 0.f );
		for (int i = STEP3START; i < STEP4START; i++)
			lights[GATE_LIGHT + i].setBrightness( (stepCount >= 3 ) ? 1.f : 0.f );
		for (int i = STEP4START; i < STEP5START; i++)
			lights[GATE_LIGHT + i].setBrightness( (stepCount >= 4 ) ? 1.f : 0.f );
		for (int i = STEP5START; i < STEP6START; i++)
			lights[GATE_LIGHT + i].setBrightness( (stepCount >= 5 ) ? 1.f : 0.f );
		for (int i = STEP6START; i < STEP7START; i++)
			lights[GATE_LIGHT + i].setBrightness( (stepCount >= 6 ) ? 1.f : 0.f );
		for (int i = STEP7START; i < STEP8START; i++)
			lights[GATE_LIGHT + i].setBrightness( (stepCount >= 7 ) ? 1.f : 0.f );
		for (int i = STEP8START; i < STEP9START; i++){
			lights[GATE_LIGHT + i].setBrightness( (stepCount >= 8 ) ? 1.f : 0.f );
		}
	}

	void onReset() override {
		step = 0;
		node = 0;
		lastNode = 0;
		pathTraveled[0] = 0;
		for (int i = 1; i < 8; i++) pathTraveled[i] = -1;
		lightsReset = true;
	}

	void process(const ProcessArgs& args) override {
		if (randomizeCvTrigger.process(params[RANDCV_PARAM].getValue())) randomizeCv(args);
		if (randomizeRouteTrigger.process(params[RANDROUTE_PARAM].getValue())) randomizeRoute(args);
		processReset(args);
		processRunStatus(args);
		processStepNumber(args);
		if (steppedForward) processNodeForward(args);
		if (steppedBack) processNodeBack(args);
		processGateOutput(args);
		processVoltageOutput(args);
		processLights(args);	
	}
};


struct AriaKnob820Random : AriaKnob820 {
	AriaKnob820Random() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/knob-820-arrow.svg")));
		minAngle = 0.25 * M_PI;
		maxAngle = 0.75 * M_PI;
	}
};

struct AriaKnob820Snap : AriaKnob820 {
	AriaKnob820Snap() {
		snap = true;
	}
};


struct DariusWidget : ModuleWidget {
	DariusWidget(Darius* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Darius.svg")));
		
		// Signature.
		addChild(createWidget<AriaSignature>(mm2px(Vec(117.5, 114.538))));
		
		// Screws
		addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// The main area - lights, knobs and trigger outputs.
		for (int i = 0; i < 1; i++) {
			addChild(createLight<AriaInputLight>(        mm2px(Vec( 4.5, (16.0 + (6.5 * 7) + i * 13.0))), module, Darius::CV_LIGHT +    i));
			addParam(createParam<AriaKnob820Transparent>(mm2px(Vec( 4.5, (16.0 + (6.5 * 7) + i * 13.0))), module, Darius::CV_PARAM +    i));
			addParam(createParam<AriaKnob820Random>(     mm2px(Vec(14.5, (16.0 + (6.5 * 7) + i * 13.0))), module, Darius::ROUTE_PARAM + i));
			addChild(createLight<AriaOutputLight>(       mm2px(Vec( 9.5, (22.5 + (6.5 * 7) + i * 13.0))), module, Darius::GATE_LIGHT +  i));
			addOutput(createOutput<AriaJackTransparent>( mm2px(Vec( 9.5, (22.5 + (6.5 * 7) + i * 13.0))), module, Darius::GATE_OUTPUT + i));
		}
		for (int i = 0; i < 2; i++) {
			addChild(createLight<AriaInputLight>(        mm2px(Vec(24.5, (16.0 + (6.5 * 6) + i * 13.0))), module, Darius::CV_LIGHT +    i + STEP2START));
			addParam(createParam<AriaKnob820Transparent>(mm2px(Vec(24.5, (16.0 + (6.5 * 6) + i * 13.0))), module, Darius::CV_PARAM +    i + STEP2START));
			addParam(createParam<AriaKnob820Random>(     mm2px(Vec(34.5, (16.0 + (6.5 * 6) + i * 13.0))), module, Darius::ROUTE_PARAM + i + STEP2START));
			addChild(createLight<AriaOutputLight>(       mm2px(Vec(29.5, (22.5 + (6.5 * 6) + i * 13.0))), module, Darius::GATE_LIGHT +  i + STEP2START));
			addOutput(createOutput<AriaJackTransparent>( mm2px(Vec(29.5, (22.5 + (6.5 * 6) + i * 13.0))), module, Darius::GATE_OUTPUT + i + STEP2START));
		}
		for (int i = 0; i < 3; i++) {
			addChild(createLight<AriaInputLight>(        mm2px(Vec(44.5, (16.0 + (6.5 * 5) + i * 13.0))), module, Darius::CV_LIGHT +    i + STEP3START));
			addParam(createParam<AriaKnob820Transparent>(mm2px(Vec(44.5, (16.0 + (6.5 * 5) + i * 13.0))), module, Darius::CV_PARAM +    i + STEP3START));
			addParam(createParam<AriaKnob820Random>(     mm2px(Vec(54.5, (16.0 + (6.5 * 5) + i * 13.0))), module, Darius::ROUTE_PARAM + i + STEP3START));
			addChild(createLight<AriaOutputLight>(       mm2px(Vec(49.5, (22.5 + (6.5 * 5) + i * 13.0))), module, Darius::GATE_LIGHT +  i + STEP3START));
			addOutput(createOutput<AriaJackTransparent>( mm2px(Vec(49.5, (22.5 + (6.5 * 5) + i * 13.0))), module, Darius::GATE_OUTPUT + i + STEP3START));
		}
		for (int i = 0; i < 4; i++) {
			addChild(createLight<AriaInputLight>(        mm2px(Vec(64.5, (16.0 + (6.5 * 4) + i * 13.0))), module, Darius::CV_LIGHT +    i + STEP4START));
			addParam(createParam<AriaKnob820Transparent>(mm2px(Vec(64.5, (16.0 + (6.5 * 4) + i * 13.0))), module, Darius::CV_PARAM +    i + STEP4START));
			addParam(createParam<AriaKnob820Random>(     mm2px(Vec(74.5, (16.0 + (6.5 * 4) + i * 13.0))), module, Darius::ROUTE_PARAM + i + STEP4START));
			addChild(createLight<AriaOutputLight>(       mm2px(Vec(69.5, (22.5 + (6.5 * 4) + i * 13.0))), module, Darius::GATE_LIGHT +  i + STEP4START));
			addOutput(createOutput<AriaJackTransparent>( mm2px(Vec(69.5, (22.5 + (6.5 * 4) + i * 13.0))), module, Darius::GATE_OUTPUT + i + STEP4START));
		}
		for (int i = 0; i < 5; i++) {
			addChild(createLight<AriaInputLight>(        mm2px(Vec(84.5, (16.0 + (6.5 * 3) + i * 13.0))), module, Darius::CV_LIGHT +    i + STEP5START));
			addParam(createParam<AriaKnob820Transparent>(mm2px(Vec(84.5, (16.0 + (6.5 * 3) + i * 13.0))), module, Darius::CV_PARAM +    i + STEP5START));
			addParam(createParam<AriaKnob820Random>(     mm2px(Vec(94.5, (16.0 + (6.5 * 3) + i * 13.0))), module, Darius::ROUTE_PARAM + i + STEP5START));
			addChild(createLight<AriaOutputLight>(       mm2px(Vec(89.5, (22.5 + (6.5 * 3) + i * 13.0))), module, Darius::GATE_LIGHT +  i + STEP5START));
			addOutput(createOutput<AriaJackTransparent>( mm2px(Vec(89.5, (22.5 + (6.5 * 3) + i * 13.0))), module, Darius::GATE_OUTPUT + i + STEP5START));
		}
		for (int i = 0; i < 6; i++) {
			addChild(createLight<AriaInputLight>(        mm2px(Vec(104.5, (16.0 + (6.5 * 2) + i * 13.0))), module, Darius::CV_LIGHT +    i + STEP6START));
			addParam(createParam<AriaKnob820Transparent>(mm2px(Vec(104.5, (16.0 + (6.5 * 2) + i * 13.0))), module, Darius::CV_PARAM +    i + STEP6START));
			addParam(createParam<AriaKnob820Random>(     mm2px(Vec(114.5, (16.0 + (6.5 * 2) + i * 13.0))), module, Darius::ROUTE_PARAM + i + STEP6START));
			addChild(createLight<AriaOutputLight>(       mm2px(Vec(109.5, (22.5 + (6.5 * 2) + i * 13.0))), module, Darius::GATE_LIGHT +  i + STEP6START));
			addOutput(createOutput<AriaJackTransparent>( mm2px(Vec(109.5, (22.5 + (6.5 * 2) + i * 13.0))), module, Darius::GATE_OUTPUT + i + STEP6START));
		}
		for (int i = 0; i < 7; i++) {
			addChild(createLight<AriaInputLight>(        mm2px(Vec(124.5, (16.0 + (6.5 * 1) + i * 13.0))), module, Darius::CV_LIGHT +    i + STEP7START));
			addParam(createParam<AriaKnob820Transparent>(mm2px(Vec(124.5, (16.0 + (6.5 * 1) + i * 13.0))), module, Darius::CV_PARAM +    i + STEP7START));
			addParam(createParam<AriaKnob820Random>(     mm2px(Vec(134.5, (16.0 + (6.5 * 1) + i * 13.0))), module, Darius::ROUTE_PARAM + i + STEP7START));
			addChild(createLight<AriaOutputLight>(       mm2px(Vec(129.5, (22.5 + (6.5 * 1) + i * 13.0))), module, Darius::GATE_LIGHT +  i + STEP7START));
			addOutput(createOutput<AriaJackTransparent>( mm2px(Vec(129.5, (22.5 + (6.5 * 1) + i * 13.0))), module, Darius::GATE_OUTPUT + i + STEP7START));
		}
		for (int i = 0; i < 8; i++) {
			addChild(createLight<AriaInputLight>(        mm2px(Vec(144.5, (16.0 + (6.5 * 0) + i * 13.0))), module, Darius::CV_LIGHT +    i + STEP8START));
			addParam(createParam<AriaKnob820Transparent>(mm2px(Vec(144.5, (16.0 + (6.5 * 0) + i * 13.0))), module, Darius::CV_PARAM +    i + STEP8START));
			addChild(createLight<AriaOutputLight>(       mm2px(Vec(149.5, (22.5 + (6.5 * 0) + i * 13.0))), module, Darius::GATE_LIGHT +  i + STEP8START));
			addOutput(createOutput<AriaJackTransparent>( mm2px(Vec(149.5, (22.5 + (6.5 * 0) + i * 13.0))), module, Darius::GATE_OUTPUT + i + STEP8START));
		}
		
		// Step < ^ v >
		addInput(createInput<AriaJackIn>(mm2px(Vec(4.5, 22.5)), module, Darius::STEP_BACK_INPUT));
		addInput(createInput<AriaJackIn>(mm2px(Vec(14.5, 18.0)), module, Darius::STEP_UP_INPUT));
		addInput(createInput<AriaJackIn>(mm2px(Vec(14.5, 27.0)), module, Darius::STEP_DOWN_INPUT));
		addInput(createInput<AriaJackIn>(mm2px(Vec(24.5, 22.5)), module, Darius::STEP_INPUT));
		addParam(createParam<AriaPushButton820Momentary>(mm2px(Vec(24.5, 32.5)), module, Darius::STEP_PARAM));
		
		// Run
		addInput(createInput<AriaJackIn>(mm2px(Vec(4.5, 42.5)), module, Darius::RUN_INPUT));
		addParam(createParam<AriaPushButton820>(mm2px(Vec(14.5, 42.5)), module, Darius::RUN_PARAM));
		
		// Reset
		addInput(createInput<AriaJackIn>(mm2px(Vec(24.5, 42.5)), module, Darius::RESET_INPUT));
		addParam(createParam<AriaPushButton820Momentary>(mm2px(Vec(34.5, 42.5)), module, Darius::RESET_PARAM));
		
		// Step count
		addParam(createParam<AriaKnob820Snap>(mm2px(Vec(54.5, 22.5)), module, Darius::STEPCOUNT_PARAM));
		
		// Randomize
		addParam(createParam<AriaPushButton820Momentary>(mm2px(Vec(64.5, 22.5)), module, Darius::RANDCV_PARAM));
		addParam(createParam<AriaPushButton820Momentary>(mm2px(Vec(74.5, 22.5)), module, Darius::RANDROUTE_PARAM));
		
		// Output
		addParam(createParam<AriaRockerSwitchVertical800>(mm2px(Vec(3.0, 87.5)), module, Darius::RANGE_PARAM));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(9.5, 87.5)), module, Darius::CV_OUTPUT));
		
		// Seed
		addParam(createParam<AriaRockerSwitchVertical800>(mm2px(Vec(3.0, 110.0)), module, Darius::SEED_MODE_PARAM));
		addInput(createInput<AriaJackIn>(mm2px(Vec(9.5, 110.0)), module, Darius::SEED_INPUT));

	}
};


Model* modelDarius = createModel<Darius, DariusWidget>("Darius");
