#include "plugin.hpp"
#include "prng.hpp"
#include "quantizer.hpp"
#include "lcd.hpp"

// Warning - this module was created with very little C++ experience, and features were 
// added to it later without regard for code quality. This is maintained exploratory code, not good design.

// TODO
// - Portable sequences
// - "New point of departure" mode
// - Quantized slides would be fun! (But maybe use another module? idk)


const int STEP1START = 0;  //               00        
const int STEP2START = 1;  //             02  01            
const int STEP3START = 3;  //           05  04  03          
const int STEP4START = 6;  //         09  08  07  06        
const int STEP5START = 10; //       14  13  12  11  10      
const int STEP6START = 15; //     20  19  18  17  16  15    
const int STEP7START = 21; //   27  26  25  24  23  22  21  
const int STEP8START = 28; // 35  34  33  32  31  30  29  28
const int STEP9START = 36; // (Panel is rotated 90 degrees counter-clockwise compared to this diagram)

const int DISPLAYDIVIDER = 512;
const int KNOBDIVIDER = 512;

enum LcdModes {
	INIT_MODE,
	DEFAULT_MODE,
	SCALE_MODE,
	KNOB_MODE,
	QUANTIZED_MODE,
	CV_MODE,
	MINMAX_MODE,
	ROUTE_MODE,
	SLIDE_MODE
};

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
		STEPFIRST_PARAM,
		MIN_PARAM,
		MAX_PARAM,
		SLIDE_PARAM,
		QUANTIZE_TOGGLE_PARAM,
		KEY_PARAM,
		SCALE_PARAM, // 1.5.0 release
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
		EXT_SCALE_INPUT, // 1.5.0 release
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(GATE_OUTPUT, 36),
		CV_OUTPUT, // 1.2.0 release
		GLOBAL_GATE_OUTPUT, // 1.5.0 release
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(CV_LIGHT, 36),
		ENUMS(GATE_LIGHT, 36), // 1.2.0 release
		SEED_LIGHT,
		NUM_LIGHTS
	};
	
	bool running = true;
	bool steppedForward = false;
	bool steppedBack = false;
	bool forceUp = false;
	bool forceDown = false;
	bool lightsReset = false;
	bool resetCV = false;
	bool resetRoutes = false;
	bool routesToTop = false;
	bool routesToBottom = false;
	bool routesToEqualProbability = false;
	bool routesToBinaryTree = false;
	std::array<bool, 12> scale;
	std::array<bool, 12> pianoDisplay;
	int stepFirst = 1;
	int stepLast = 8;
	int step = 0;
	int node = 0;
	int lastNode = 0;
	int lastGate = 0;
	int pathTraveled[8] = { 0, -1, -1, -1, -1, -1, -1, -1 }; // -1 = not gone there yet
	int lcdMode = INIT_MODE;
	int lastCvChanged = 0;
	int lastRouteChanged = 0;
	float randomSeed = 0.f;
	float slideDuration = 0.f; // In ms
	float slideCounter = 0.f;
	float lastOutput = 0.f;
	float lcdLastInteraction = 0.f;
	float probabilities[36];
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
	dsp::PulseGenerator manualStepTrigger;
	dsp::ClockDivider knobDivider;
	dsp::ClockDivider displayDivider;
	prng::prng prng;
	Lcd::LcdStatus lcdStatus;

	Darius() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(STEP_PARAM, 0.f, 1.f, 0.f, "Step");
		configParam(RUN_PARAM, 0.f, 1.f, 1.f, "Run");
		configParam(RESET_PARAM, 0.f, 1.f, 0.f, "Reset");
		configParam(STEPFIRST_PARAM, 1.f, 8.f, 1.f, "First step");
		configParam(STEPCOUNT_PARAM, 1.f, 8.f, 8.f, "Last step");
		configParam(RANDCV_PARAM, 0.f, 1.f, 0.f, "Randomize CV knobs");
		configParam(RANDROUTE_PARAM, 0.f, 1.f, 0.f, "Meta-randomize random route knobs");
		configParam(SEED_MODE_PARAM, 0.f, 1.f, 0.f, "New random seed on first or all nodes");
		configParam(RANGE_PARAM, 0.f, 1.f, 0.f, "Voltage output range");
		configParam(MIN_PARAM, 0.f, 10.f, 0.f, "Minimum CV/Note");
		configParam(MAX_PARAM, 0.f, 10.f, 10.f, "Maximum CV/Note");
		configParam(QUANTIZE_TOGGLE_PARAM, 0.f, 1.f, 0.f, "Precise CV/Quantized V/Oct");
		configParam(KEY_PARAM, 0.f, 11.f, 0.f, "Key");
		configParam(SCALE_PARAM, 0.f, (float) Quantizer::NUM_SCALES - 1, 0.f, "Scale");
		configParam(SLIDE_PARAM, 0.f, 10.f, 0.f, "Slide");
		for (int i = 0; i < STEP9START; i++)
			configParam(CV_PARAM + i, 0.f, 10.f, 5.f, "CV");
		for (int i = 0; i < STEP8START; i++)
			configParam(ROUTE_PARAM + i, 0.f, 1.f, 0.5f, "Random route");
		knobDivider.setDivision(KNOBDIVIDER); 
		displayDivider.setDivision(DISPLAYDIVIDER);
		lcdStatus.lcdPage = Lcd::TEXT1_AND_TEXT2_MODE;
		lcdStatus.lcdText1 = "MEDITATE..."; // Loading message
		lcdStatus.lcdText2 = "MEDITATION."; // https://www.youtube.com/watch?v=JqLNY1zyQ6o
		for (int i = 0; i < 100; i++) random::uniform(); // The first few seeds we get seem bad, need more warming up. Might just be superstition.
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


	// Undo/Redo for Randomize buttons and Reset right click options.
	// Thanks to David O'Rourke for the example implementation!
	// https://github.com/AriaSalvatrice/AriaVCVModules/issues/14
	struct BulkCvAction : history::ModuleAction {
		std::array<float, 36> oldValues;
		std::array<float, 36> newValues;
		int param;

		BulkCvAction(int moduleId, std::string name, int param, std::array<float, 36> oldValues, std::array<float, 36> newValues) {
			this->moduleId = moduleId;
			this->name = name;
			this->param = param;
			this->oldValues = oldValues;
			this->newValues = newValues;
		}

		void undo() override {
			Darius *module = dynamic_cast<Darius*>(APP->engine->getModule(this->moduleId));
			if (module) {
				for (int i = 0; i < 36; i++) module->params[param + i].setValue(this->oldValues[i]);
			}
		}

		void redo() override {
			Darius *module = dynamic_cast<Darius*>(APP->engine->getModule(this->moduleId));
			if (module) {
				for (int i = 0; i < 36; i++) module->params[param + i].setValue(this->newValues[i]);
			}
		}
	};

	void randomizeCv(const ProcessArgs& args){
		std::array<float, 36> oldValues;
		std::array<float, 36> newValues;
		for (int i = 0; i < 36; i++) oldValues[i] = params[CV_PARAM + i].getValue();
		for (int i = 0; i < 36; i++) params[CV_PARAM + i].setValue(random::uniform() * 10.f);
		for (int i = 0; i < 36; i++) newValues[i] = params[CV_PARAM + i].getValue();
		APP->history->push(new BulkCvAction(this->id, "randomize Darius CV", CV_PARAM, oldValues, newValues));
	}
	
	void randomizeRoute(const ProcessArgs& args){
		std::array<float, 36> oldValues;
		std::array<float, 36> newValues;
		for (int i = 0; i < 36; i++) oldValues[i] = params[ROUTE_PARAM + i].getValue();
		for (int i = 0; i < 36; i++) params[ROUTE_PARAM + i].setValue(random::uniform());	
		for (int i = 0; i < 36; i++) newValues[i] = params[ROUTE_PARAM + i].getValue();
		APP->history->push(new BulkCvAction(this->id, "randomize Darius Routes", ROUTE_PARAM, oldValues, newValues));
	}
	
	void processResetCV(const ProcessArgs& args){
		resetCV = false;
		std::array<float, 36> oldValues;
		std::array<float, 36> newValues;
		for (int i = 0; i < 36; i++) oldValues[i] = params[CV_PARAM + i].getValue();
		for (int i = 0; i < 36; i++) params[CV_PARAM + i].setValue(5.f);	
		for (int i = 0; i < 36; i++) newValues[i] = params[CV_PARAM + i].getValue();
		APP->history->push(new BulkCvAction(this->id, "reset Darius CV", CV_PARAM, oldValues, newValues));
	}

	void processResetRoutes(const ProcessArgs& args){
		resetRoutes = false;
		std::array<float, 36> oldValues;
		std::array<float, 36> newValues;
		for (int i = 0; i < 36; i++) oldValues[i] = params[ROUTE_PARAM + i].getValue();
		for (int i = 0; i < 36; i++) params[ROUTE_PARAM + i].setValue(0.5f);	
		for (int i = 0; i < 36; i++) newValues[i] = params[ROUTE_PARAM + i].getValue();
		APP->history->push(new BulkCvAction(this->id, "reset Darius Routes", ROUTE_PARAM, oldValues, newValues));
	}

	void processRoutesToTop(const ProcessArgs& args){
		routesToTop = false;
		std::array<float, 36> oldValues;
		std::array<float, 36> newValues;
		for (int i = 0; i < 36; i++) oldValues[i] = params[ROUTE_PARAM + i].getValue();
		for (int i = 0; i < 36; i++) params[ROUTE_PARAM + i].setValue(0.f);	
		for (int i = 0; i < 36; i++) newValues[i] = params[ROUTE_PARAM + i].getValue();
		APP->history->push(new BulkCvAction(this->id, "set Darius Routes to Top", ROUTE_PARAM, oldValues, newValues));
	}

	void processRoutesToBottom(const ProcessArgs& args){
		routesToBottom = false;
		std::array<float, 36> oldValues;
		std::array<float, 36> newValues;
		for (int i = 0; i < 36; i++) oldValues[i] = params[ROUTE_PARAM + i].getValue();
		for (int i = 0; i < 36; i++) params[ROUTE_PARAM + i].setValue(1.f);	
		for (int i = 0; i < 36; i++) newValues[i] = params[ROUTE_PARAM + i].getValue();
		APP->history->push(new BulkCvAction(this->id, "set Darius Routes to Bottom", ROUTE_PARAM, oldValues, newValues));
	}

	void processRoutesToEqualProbability(const ProcessArgs& args){
		routesToEqualProbability = false;
		std::array<float, 36> oldValues;
		std::array<float, 36> newValues;
		for (int i = 0; i < 36; i++) oldValues[i] = params[ROUTE_PARAM + i].getValue();
		params[ROUTE_PARAM].setValue(0.5f);
		for (int i = 0; i < 2; i++) params[ROUTE_PARAM + i + STEP2START].setValue( (i + 1) / 3.f );
		for (int i = 0; i < 3; i++) params[ROUTE_PARAM + i + STEP3START].setValue( (i + 1) / 4.f );
		for (int i = 0; i < 4; i++) params[ROUTE_PARAM + i + STEP4START].setValue( (i + 1) / 5.f );
		for (int i = 0; i < 5; i++) params[ROUTE_PARAM + i + STEP5START].setValue( (i + 1) / 6.f );
		for (int i = 0; i < 6; i++) params[ROUTE_PARAM + i + STEP6START].setValue( (i + 1) / 7.f );
		for (int i = 0; i < 7; i++) params[ROUTE_PARAM + i + STEP7START].setValue( (i + 1) / 8.f );
		for (int i = 0; i < 36; i++) newValues[i] = params[ROUTE_PARAM + i].getValue();
		APP->history->push(new BulkCvAction(this->id, "set Darius Routes to Spread out", ROUTE_PARAM, oldValues, newValues));
	}

	// Thanks to stoermelder for the idea!
	// https://community.vcvrack.com/t/arias-cool-and-nice-thread-of-barely-working-betas-and-bug-squashing-darius-update/8208/13?u=aria_salvatrice
	void processRoutesToBinaryTree(const ProcessArgs& args){
		routesToBinaryTree = false;
		std::array<float, 36> oldValues;
		std::array<float, 36> newValues;
		for (int i = 0; i < 36; i++) oldValues[i] = params[ROUTE_PARAM + i].getValue();
		for (int i = 0; i < 36; i++) params[ROUTE_PARAM + i].setValue(0.5f);
		params[ROUTE_PARAM +  1].setValue(0.f);
		params[ROUTE_PARAM +  2].setValue(1.f);
		params[ROUTE_PARAM +  6].setValue(0.f);
		params[ROUTE_PARAM +  7].setValue(0.f);
		params[ROUTE_PARAM +  8].setValue(1.f);
		params[ROUTE_PARAM +  9].setValue(1.f);
		params[ROUTE_PARAM + 10].setValue(0.f);
		params[ROUTE_PARAM + 11].setValue(1.f);
		params[ROUTE_PARAM + 13].setValue(0.f);
		params[ROUTE_PARAM + 14].setValue(1.f);
		params[ROUTE_PARAM + 15].setValue(0.f);
		params[ROUTE_PARAM + 17].setValue(0.f);
		params[ROUTE_PARAM + 18].setValue(1.f);
		params[ROUTE_PARAM + 20].setValue(1.f);
		for (int i = 0; i < 36; i++) newValues[i] = params[ROUTE_PARAM + i].getValue();
		APP->history->push(new BulkCvAction(this->id, "set Darius Routes to Binary tree", ROUTE_PARAM, oldValues, newValues));
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

	// Reset to the first step
	void reset(const ProcessArgs& args){
		step = 0;
		node = 0;
		lastNode = 0;
		lightsReset = true;
		resetPathTraveled(args);
		for (int i = 0; i < 36; i++)
			outputs[GATE_OUTPUT + i].setVoltage(0.f);
		lcdStatus.lcdDirty = true;
	}
	
	// Sets running to the current run status
	void setRunStatus(const ProcessArgs& args){
		if (runCvTrigger.process(inputs[RUN_INPUT].getVoltageSum())){
			running = !running;
			params[RUN_PARAM].setValue(running);
		}
		running = params[RUN_PARAM].getValue();
	}
		
	void setStepStatus(const ProcessArgs& args){
		stepFirst = std::round(params[STEPFIRST_PARAM].getValue());
		stepLast  = std::round(params[STEPCOUNT_PARAM].getValue());
		if (stepFirst > stepLast) stepFirst = stepLast;
		if (running) {
			bool triggerAccepted = false; // Accept only one trigger!
			if (stepForwardCvTrigger.process(inputs[STEP_INPUT].getVoltageSum())){
				step++;
				steppedForward = true;
				triggerAccepted = true;
				slideCounter = 0.f;
				lastOutput = outputs[CV_OUTPUT].getVoltage();
			}
			if (stepUpCvTrigger.process(inputs[STEP_UP_INPUT].getVoltageSum()) and !triggerAccepted){
				step++;
				forceUp = true;
				steppedForward = true;
				triggerAccepted = true;
				slideCounter = 0.f;
				lastOutput = outputs[CV_OUTPUT].getVoltage();
			}
			if (stepDownCvTrigger.process(inputs[STEP_DOWN_INPUT].getVoltageSum()) and !triggerAccepted){
				step++;
				forceDown = true;
				steppedForward = true;
				triggerAccepted = true;
				slideCounter = 0.f;
				lastOutput = outputs[CV_OUTPUT].getVoltage();
			}
			if (stepBackCvTrigger.process(inputs[STEP_BACK_INPUT].getVoltageSum()) and step > 0 and !triggerAccepted){
				step--;
				steppedBack = true;
				slideCounter = 0.f;
				lastOutput = outputs[CV_OUTPUT].getVoltage();
			}
		}
		if (stepForwardButtonTrigger.process(params[STEP_PARAM].getValue())){
			step++; // You can still advance manually if module isn't running
			steppedForward = true;
			slideCounter = 0.f;
			lastOutput = outputs[CV_OUTPUT].getVoltage();
			manualStepTrigger.trigger(1e-3f);
		}
		lastGate = node;
		if (step >= stepLast || step < stepFirst - 1) {
			step = 0;
			node = 0;
			lastNode = 0;
			resetPathTraveled(args);
			lightsReset = true;
			slideCounter = 0.f;
			lastOutput = outputs[CV_OUTPUT].getVoltage();
			for(int i = 0; i < stepFirst - 1; i++) {
				step++;
				nodeForward(args);
			}
		}
	}

	int getUpChild(int parent){
		if (parent == 0) return 1;
		if (parent == 1) return 3;
		if (parent == 2) return 4;
		if (parent >= 3 && parent <= 5)   return parent + 3;
		if (parent >= 6 && parent <= 9)   return parent + 4;
		if (parent >= 10 && parent <= 14) return parent + 5;
		if (parent >= 15 && parent <= 20) return parent + 6;
		if (parent >= 21 && parent <= 27) return parent + 7;
		return 0;
	}

	int getDownChild(int parent){
		return getUpChild(parent) + 1;
	}

	void updateRoutes(const ProcessArgs& args){
		// This is hard to think about, so I did it by hand, lol
		probabilities[0]  = 1.f;

		probabilities[1]  = 1.f - params[ROUTE_PARAM + 0].getValue();
		probabilities[2]  = params[ROUTE_PARAM + 0].getValue();

		probabilities[3]  = probabilities[1] *  (1.f - params[ROUTE_PARAM + 1].getValue());
		probabilities[4]  = (probabilities[1] * params[ROUTE_PARAM + 1].getValue()) + (probabilities[2] * (1.f - params[ROUTE_PARAM + 2].getValue()));
		probabilities[5]  = probabilities[2] * params[ROUTE_PARAM + 2].getValue();

		probabilities[6]  = probabilities[3] *  (1.f - params[ROUTE_PARAM + 3].getValue());
		probabilities[7]  = (probabilities[3] * params[ROUTE_PARAM + 3].getValue()) + (probabilities[4] * (1.f - params[ROUTE_PARAM + 4].getValue()));
		probabilities[8]  = (probabilities[4] * params[ROUTE_PARAM + 4].getValue()) + (probabilities[5] * (1.f - params[ROUTE_PARAM + 5].getValue()));
		probabilities[9]  = probabilities[5] * params[ROUTE_PARAM + 5].getValue();

		probabilities[10] = probabilities[6] *  (1.f - params[ROUTE_PARAM + 6].getValue());
		probabilities[11] = (probabilities[6] * params[ROUTE_PARAM + 6].getValue()) + (probabilities[7] * (1.f - params[ROUTE_PARAM + 7].getValue()));
		probabilities[12] = (probabilities[7] * params[ROUTE_PARAM + 7].getValue()) + (probabilities[8] * (1.f - params[ROUTE_PARAM + 8].getValue()));
		probabilities[13] = (probabilities[8] * params[ROUTE_PARAM + 8].getValue()) + (probabilities[9] * (1.f - params[ROUTE_PARAM + 9].getValue()));
		probabilities[14] = probabilities[9] * params[ROUTE_PARAM + 9].getValue();

		probabilities[15] = probabilities[10] *  (1.f - params[ROUTE_PARAM + 10].getValue());
		probabilities[16] = (probabilities[10] * params[ROUTE_PARAM + 10].getValue()) + (probabilities[11] * (1.f - params[ROUTE_PARAM + 11].getValue()));
		probabilities[17] = (probabilities[11] * params[ROUTE_PARAM + 11].getValue()) + (probabilities[12] * (1.f - params[ROUTE_PARAM + 12].getValue()));
		probabilities[18] = (probabilities[12] * params[ROUTE_PARAM + 12].getValue()) + (probabilities[13] * (1.f - params[ROUTE_PARAM + 13].getValue()));
		probabilities[19] = (probabilities[13] * params[ROUTE_PARAM + 13].getValue()) + (probabilities[14] * (1.f - params[ROUTE_PARAM + 14].getValue()));
		probabilities[20] = probabilities[14] * params[ROUTE_PARAM + 14].getValue();

		probabilities[21] = probabilities[15] *  (1.f - params[ROUTE_PARAM + 15].getValue());
		probabilities[22] = (probabilities[15] * params[ROUTE_PARAM + 15].getValue()) + (probabilities[16] * (1.f - params[ROUTE_PARAM + 16].getValue()));
		probabilities[23] = (probabilities[16] * params[ROUTE_PARAM + 16].getValue()) + (probabilities[17] * (1.f - params[ROUTE_PARAM + 17].getValue()));
		probabilities[24] = (probabilities[17] * params[ROUTE_PARAM + 17].getValue()) + (probabilities[18] * (1.f - params[ROUTE_PARAM + 18].getValue()));
		probabilities[25] = (probabilities[18] * params[ROUTE_PARAM + 18].getValue()) + (probabilities[19] * (1.f - params[ROUTE_PARAM + 19].getValue()));
		probabilities[26] = (probabilities[19] * params[ROUTE_PARAM + 19].getValue()) + (probabilities[20] * (1.f - params[ROUTE_PARAM + 20].getValue()));
		probabilities[27] = probabilities[20] * params[ROUTE_PARAM + 20].getValue();

		probabilities[28] = probabilities[21] *  (1.f - params[ROUTE_PARAM + 21].getValue());
		probabilities[29] = (probabilities[21] * params[ROUTE_PARAM + 21].getValue()) + (probabilities[22] * (1.f - params[ROUTE_PARAM + 22].getValue()));
		probabilities[30] = (probabilities[22] * params[ROUTE_PARAM + 22].getValue()) + (probabilities[23] * (1.f - params[ROUTE_PARAM + 23].getValue()));
		probabilities[31] = (probabilities[23] * params[ROUTE_PARAM + 23].getValue()) + (probabilities[24] * (1.f - params[ROUTE_PARAM + 24].getValue()));
		probabilities[32] = (probabilities[24] * params[ROUTE_PARAM + 24].getValue()) + (probabilities[25] * (1.f - params[ROUTE_PARAM + 25].getValue()));
		probabilities[33] = (probabilities[25] * params[ROUTE_PARAM + 25].getValue()) + (probabilities[26] * (1.f - params[ROUTE_PARAM + 26].getValue()));
		probabilities[34] = (probabilities[26] * params[ROUTE_PARAM + 26].getValue()) + (probabilities[27] * (1.f - params[ROUTE_PARAM + 27].getValue()));
		probabilities[35] = probabilities[27] * params[ROUTE_PARAM + 27].getValue();
	}

	// From 1ms to 10s. 
	// Using this code as reference:
	// https://github.com/mgunyho/Little-Utils/blob/master/src/PulseGenerator.cpp
	// This has a bit of a performance impact so it's not called every sample.
	void setSlide(const ProcessArgs& args){
		slideDuration = params[SLIDE_PARAM].getValue();
		if (slideDuration > 0.00001f ) {
			slideDuration = rescale(slideDuration, 0.f, 10.f, -3.0f, 1.0f);
			slideDuration = powf(10.0f, slideDuration);
		} else {
			slideDuration = 0.f;
		}
	}
	
	void nodeForward(const ProcessArgs& args){
		steppedForward = false;
		
		// Refresh seed at start of sequences, and on external seeds
		// Refresh at the last minute: when about to move the second step (step == 1), not when entering the first (step == 0).
		if (step == 1){
			refreshSeed(args);
			prng.init(randomSeed, randomSeed);
		} else {
			if (params[SEED_MODE_PARAM].getValue() == 1.0f && inputs[SEED_INPUT].isConnected()){
				refreshSeed(args);
				prng.init(randomSeed, randomSeed);
			}
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
				if (prng.uniform() < params[ROUTE_PARAM + lastNode].getValue()) {
					node = node + step + 1;
				} else {
					node = node + step;
				}
				
			}
		}
		pathTraveled[step] = node;
		lastNode = node;
		lcdStatus.lcdDirty = true;
	}
	
	void nodeBack(const ProcessArgs& args){
		lightsReset = true;
		node = pathTraveled[step];
		// FIXME - This conditional avoids a bizarre problem where randomSeed goes NaN. Not sure what's exactly going on!!
		if (step < 7) pathTraveled[step + 1] = -1; 
		lastNode = node;
		lcdStatus.lcdDirty = true;
	}

	void updateScale(const ProcessArgs& args){
		if (inputs[EXT_SCALE_INPUT].isConnected()) {
			for(int i = 0; i < 12; i++) {
				scale [i] = (inputs[EXT_SCALE_INPUT].getVoltage(i) > 0.f) ? true : false;
			}
		} else {
			scale = Quantizer::validNotesInScaleKey( (int)params[SCALE_PARAM].getValue() , (int)params[KEY_PARAM].getValue() );
		}
	}
	
	void sendGateOutput(const ProcessArgs& args){

		bool manualStep = manualStepTrigger.process(args.sampleTime);

		if (inputs[STEP_INPUT].isConnected() || inputs[STEP_BACK_INPUT].isConnected() || inputs[STEP_UP_INPUT].isConnected() || inputs[STEP_DOWN_INPUT].isConnected()){
			float output;
			output = inputs[STEP_INPUT].getVoltageSum();
			output = (inputs[STEP_BACK_INPUT].getVoltageSum() > output) ? inputs[STEP_BACK_INPUT].getVoltageSum() : output;
			output = (inputs[STEP_UP_INPUT].getVoltageSum() > output)   ? inputs[STEP_UP_INPUT].getVoltageSum()   : output;
			output = (inputs[STEP_DOWN_INPUT].getVoltageSum() > output) ? inputs[STEP_DOWN_INPUT].getVoltageSum() : output;
			outputs[GATE_OUTPUT + node].setVoltage(output);
			outputs[GLOBAL_GATE_OUTPUT].setVoltage(output);
		} else {
			outputs[GATE_OUTPUT + lastGate].setVoltage(0.f);
			outputs[GATE_OUTPUT + node].setVoltage(10.f);
			outputs[GLOBAL_GATE_OUTPUT].setVoltage( (manualStep) ? 10.f : 0.f );
		}
	}

	void setVoltageOutput(const ProcessArgs& args){
		float output = params[CV_PARAM + node].getValue();

		if (params[QUANTIZE_TOGGLE_PARAM].getValue() == 0.f) {
			// When not quantized
			if (params[RANGE_PARAM].getValue() == 0.f) {
				// O~10V
				output = rescale(output, 0.f, 10.f, params[MIN_PARAM].getValue(), params[MAX_PARAM].getValue());
			} else {
				// -5~5V
				output = rescale(output, 0.f, 10.f, params[MIN_PARAM].getValue() - 5.f, params[MAX_PARAM].getValue() - 5.f);
			}
		} else {
			// When quantized start somewhere closer to what oscillators accept
			if (params[RANGE_PARAM].getValue() == 0.f) {
				output = rescale(output, 0.f, 10.f, params[MIN_PARAM].getValue() - 4.f, params[MAX_PARAM].getValue() - 4.f);
			} else {
				// -1 octave button
				output = rescale(output, 0.f, 10.f, params[MIN_PARAM].getValue() - 5.f, params[MAX_PARAM].getValue() - 5.f);
			}
			// Then quantize it
			output = Quantizer::quantize(output, scale);
		}

		// Slide
		if (slideDuration > 0.f && slideDuration > slideCounter) {
			output = crossfade(lastOutput, output, (slideCounter / slideDuration) );
			slideCounter += args.sampleTime;
		}

		outputs[CV_OUTPUT].setVoltage(output);
	}
	
	void updateLights(const ProcessArgs& args){
		// The Seed input light
		lights[SEED_LIGHT].setBrightness( ( inputs[SEED_INPUT].getVoltage() == 0.f ) ? 0.f : 1.f );


		// Clean up by request only
		if (lightsReset) {
			for (int i = 0; i < 36; i++) lights[CV_LIGHT + i].setBrightness( 0.f );
			for (int i = 0; i < 8; i++) {
				if (pathTraveled[i] >= 0) lights[CV_LIGHT + pathTraveled[i]].setBrightness( 1.f );
			}
			lightsReset = false;
		}		

		// Using an intermediary to prevent flicker
		lights[CV_LIGHT + pathTraveled[step]].setBrightness( 1.f );

		float brightness[36];
		// Light the outputs depending on amount of steps enabled
		brightness[0] = (stepFirst <= 1 && stepLast >= 1 ) ? 1.f : 0.f ;
		for (int i = STEP2START; i < STEP3START; i++)
			brightness[i] = (stepFirst <= 2 && stepLast >= 2 ) ? 1.f : 0.f;
		for (int i = STEP3START; i < STEP4START; i++)
			brightness[i] = (stepFirst <= 3 && stepLast >= 3 ) ? 1.f : 0.f;
		for (int i = STEP4START; i < STEP5START; i++)
			brightness[i] = (stepFirst <= 4 && stepLast >= 4 ) ? 1.f : 0.f;
		for (int i = STEP5START; i < STEP6START; i++)
			brightness[i] = (stepFirst <= 5 && stepLast >= 5 ) ? 1.f : 0.f;
		for (int i = STEP6START; i < STEP7START; i++)
			brightness[i] = (stepFirst <= 6 && stepLast >= 6 ) ? 1.f : 0.f;
		for (int i = STEP7START; i < STEP8START; i++)
			brightness[i] = (stepFirst <= 7 && stepLast >= 7 ) ? 1.f : 0.f;
		for (int i = STEP8START; i < STEP9START; i++)
			brightness[i] = (stepFirst <= 8 && stepLast >= 8 ) ? 1.f : 0.f;
		// And turn off nodes that are impossible to reach
		for (int i = 0; i < 36; i++)
			if (probabilities[i] == 0.f) brightness[i] = 0.f;

		for (int i = 0; i < 36; i++)
			lights[GATE_LIGHT + i].setBrightness(brightness[i]);
		
	}

	// Sets the lcdStatus according to the lcdMode.
	void updateLcd(const ProcessArgs& args){

		// Updating multiple times a variable that gets read such as lcdText2 causes crashes due to reasons.
		// Use temporary variables instead and write only once. 
		std::string text, relative, absolute;
		float f;
		std::array<bool, 12> validNotes;

		// Since we might be sliding, refresh at least this often
		lcdStatus.lcdDirty = true;

		// Reset after 3 seconds since the last interactive input was touched
		if (lcdLastInteraction < (3.f / DISPLAYDIVIDER) ) {
			lcdLastInteraction += args.sampleTime;
			// Updating only once after reset
			if(lcdLastInteraction >= (3.f / DISPLAYDIVIDER) ) {
				lcdMode = DEFAULT_MODE;
				lcdStatus.lcdDirty = true;
			}
		}

		// Default mode = pick the relevant one instead
		if(lcdMode == DEFAULT_MODE) {
			lcdMode = (params[QUANTIZE_TOGGLE_PARAM].getValue() == 0.f) ? CV_MODE : QUANTIZED_MODE;
		}

		if (lcdMode == SLIDE_MODE) {
			lcdStatus.lcdPage = Lcd::TEXT2_MODE;
			lcdStatus.lcdText1 = "SLIDE:";
			float displayDuration = slideDuration;
			if (displayDuration == 0.f)
				lcdStatus.lcdText2 = "DISABLED";
			if (displayDuration > 0.f && displayDuration < 1.f) {
				int displayDurationMs = displayDuration * 1000;
				displayDurationMs = truncf(displayDurationMs);
				lcdStatus.lcdText2 = std::to_string(displayDurationMs);
				lcdStatus.lcdText2.append("ms");
			} 
			if (displayDuration >= 1.f) {
				lcdStatus.lcdText2 = std::to_string(displayDuration);
				lcdStatus.lcdText2.resize(4);
				lcdStatus.lcdText2.append("s");
			}
		}

		if (lcdMode == SCALE_MODE) {
			lcdStatus.lcdPage = Lcd::PIANO_AND_TEXT2_MODE;
			if(params[SCALE_PARAM].getValue() == 0.f) {
				text = "CHROMATIC";
			} else {
				text = Quantizer::noteLcdName((int)params[KEY_PARAM].getValue());
				text.append(" ");
				text.append(Quantizer::scaleLcdName((int)params[SCALE_PARAM].getValue()));
			}
			if(inputs[EXT_SCALE_INPUT].isConnected()){
				text = "EXTERNAL";
			}
			lcdStatus.lcdText2 = text;
			pianoDisplay = scale;
		}

		if (lcdMode == QUANTIZED_MODE){
			lcdStatus.lcdPage = Lcd::PIANO_AND_TEXT2_MODE;
			lcdStatus.lcdText2 = Quantizer::noteName(outputs[CV_OUTPUT].getVoltage());
			pianoDisplay = Quantizer::pianoDisplay(outputs[CV_OUTPUT].getVoltage());
		}

		if (lcdMode == CV_MODE){
			lcdStatus.lcdPage = Lcd::TEXT2_MODE;
			text = std::to_string( outputs[CV_OUTPUT].getVoltage() );
			text.resize(5);
			lcdStatus.lcdText2 = text + "V";
		}

		if (lcdMode == MINMAX_MODE) {
			lcdStatus.lcdPage = Lcd::TEXT1_AND_TEXT2_MODE;
			if (params[QUANTIZE_TOGGLE_PARAM].getValue() == 0.f) {
				text = (params[RANGE_PARAM].getValue() == 0.f) ? std::to_string(params[MIN_PARAM].getValue()) : std::to_string(params[MIN_PARAM].getValue() - 5.f);
				text.resize(5);
				text.append("V");
			} else {
				text = (params[RANGE_PARAM].getValue() == 0.f) ? Quantizer::noteName(params[MIN_PARAM].getValue() - 4.f) : Quantizer::noteName(params[MIN_PARAM].getValue() - 5.f);
			}
			lcdStatus.lcdText1 = "Min: " + text;

			if (params[QUANTIZE_TOGGLE_PARAM].getValue() == 0.f) {
				text = (params[RANGE_PARAM].getValue() == 0.f) ? std::to_string(params[MAX_PARAM].getValue()) : std::to_string(params[MAX_PARAM].getValue() - 5.f);
				text.resize(5);
				text.append("V");
			} else {
				text = (params[RANGE_PARAM].getValue() == 0.f) ? Quantizer::noteName(params[MAX_PARAM].getValue() - 4.f) : Quantizer::noteName(params[MAX_PARAM].getValue() - 5.f);
			}
			lcdStatus.lcdText2 = "Max: " + text;
		}


		if (lcdMode == KNOB_MODE) {
			if (params[QUANTIZE_TOGGLE_PARAM].getValue() == 0.f) {
				lcdStatus.lcdPage = Lcd::TEXT2_MODE;
				if (params[RANGE_PARAM].getValue() == 0.f) {
					f = rescale( params[CV_PARAM + lastCvChanged].getValue(), 0.f, 10.f, params[MIN_PARAM].getValue(), params[MAX_PARAM].getValue());
				} else {
					f = rescale( params[CV_PARAM + lastCvChanged].getValue(), 0.f, 10.f, params[MIN_PARAM].getValue() - 5.f, params[MAX_PARAM].getValue() - 5.f);
				}
				text = std::to_string(f);
				text.resize(5);
				lcdStatus.lcdText2 = ">" + text + "V";
			} else {
				lcdStatus.lcdPage = Lcd::PIANO_AND_TEXT2_MODE;
				validNotes = Quantizer::validNotesInScaleKey( (int)params[SCALE_PARAM].getValue() , (int)params[KEY_PARAM].getValue() );
				 if (params[RANGE_PARAM].getValue() == 0.f) {
					 f = rescale(params[CV_PARAM + lastCvChanged].getValue(), 0.f, 10.f, params[MIN_PARAM].getValue() - 4.f, params[MAX_PARAM].getValue() - 4.f);
				 } else {
					 f = rescale(params[CV_PARAM + lastCvChanged].getValue(), 0.f, 10.f, params[MIN_PARAM].getValue() - 5.f, params[MAX_PARAM].getValue() - 5.f);
				 }
				 f = Quantizer::quantize( f, validNotes);
				 pianoDisplay = Quantizer::pianoDisplay(f);
				 lcdStatus.lcdText2 = ">" + Quantizer::noteName(f);
			}
		}

		if (lcdMode == ROUTE_MODE) {
			lcdStatus.lcdPage = Lcd::TEXT1_AND_TEXT2_MODE;
			relative = std::to_string( (1.f - params[ROUTE_PARAM + lastRouteChanged].getValue()) * 100.f);
			relative.resize(4);
			if (1.f - params[ROUTE_PARAM + lastRouteChanged].getValue() == 1.f){
				relative.resize(3);
				relative.append(" %");
			} else {
				relative.resize(4);
				relative.append("%");
			}
			if (probabilities[getUpChild(lastRouteChanged)] >= 0.1f) {
				absolute = std::to_string(roundf(probabilities[getUpChild(lastRouteChanged)] * 1000.f) / 10.f);
			} else {
				absolute = std::to_string(roundf(probabilities[getUpChild(lastRouteChanged)] * 10000.f) / 100.f);
			}
			if (probabilities[getUpChild(lastRouteChanged)] == 1.f){
				absolute.resize(3);
				absolute.append(" %");
			} else {
				absolute.resize(4);
				absolute.append("%");
			}
			lcdStatus.lcdText1 = relative + "/" + absolute;

			relative = std::to_string(params[ROUTE_PARAM + lastRouteChanged].getValue() * 100.f);
			relative.resize(4);
			if (params[ROUTE_PARAM + lastRouteChanged].getValue() == 1.f){
				relative.resize(3);
				relative.append(" %");
			} else {
				relative.resize(4);
				relative.append("%");
			}
			if (probabilities[getUpChild(lastRouteChanged)] >= 0.1f) {
				absolute = std::to_string(roundf(probabilities[getDownChild(lastRouteChanged)] * 1000.f) / 10.f);
			} else {
				absolute = std::to_string(roundf(probabilities[getDownChild(lastRouteChanged)] * 10000.f) / 100.f);
			}
			if (probabilities[getDownChild(lastRouteChanged)] == 1.f){
				absolute.resize(3);
				absolute.append(" %");
			} else {
				absolute.resize(4);
				absolute.append("%");
			}
			lcdStatus.lcdText2 = relative + "/" + absolute;
		}
	}

	void onReset() override {
		step = 0;
		node = 0;
		lastNode = 0;
		pathTraveled[0] = 0;
		for (int i = 1; i < 8; i++) pathTraveled[i] = -1;
		lightsReset = true;
		lcdMode = INIT_MODE;
		lcdStatus.lcdText1 = "MEDITATE...";
		lcdStatus.lcdText2 = "MEDITATION.";
		lcdLastInteraction = 0.f;
		lcdStatus.lcdDirty = true;
	}

	void process(const ProcessArgs& args) override {
		if (randomizeCvTrigger.process(params[RANDCV_PARAM].getValue()))
			randomizeCv(args);
		if (randomizeRouteTrigger.process(params[RANDROUTE_PARAM].getValue()))
			randomizeRoute(args);
		if (resetCvTrigger.process(inputs[RESET_INPUT].getVoltageSum()) or resetButtonTrigger.process(params[RESET_PARAM].getValue()))
			reset(args);

		if (resetCV)
			processResetCV(args);
		if (resetRoutes)
			processResetRoutes(args);
		if (routesToTop)
			processRoutesToTop(args);
		if (routesToBottom)
			processRoutesToBottom(args);
		if (routesToEqualProbability)
			processRoutesToEqualProbability(args);
		if (routesToBinaryTree)
			processRoutesToBinaryTree(args);

		setRunStatus(args);
		setStepStatus(args);

		updateRoutes(args);

		// Refreshing some non-essential knobs often has a performance impact
		// so the divider will remain quite high unless someone complains
		// it breaks their art. 
		if (knobDivider.process()) {
			setSlide(args);
		}

		if (steppedForward)
			nodeForward(args);
		if (steppedBack)
			nodeBack(args);

		updateScale(args);

		sendGateOutput(args);
		setVoltageOutput(args);
		
		if (displayDivider.process()) {
			updateLights(args);
			updateLcd(args);
		}

	}
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////


struct AriaKnob820Snap : AriaKnob820 {
	AriaKnob820Snap() {
		snap = true;
		AriaKnob820();
	}
};


// Passes the module to the created knobs
template <class TParamWidget>
TParamWidget* createLcdParam(math::Vec pos, Darius* module, int paramId) {
	TParamWidget* o = new TParamWidget(module);
	o->box.pos = pos;
	if (module) {
		o->paramQuantity = module->paramQuantities[paramId];
	}
	return o;
}

// FIXME - How do I avoid duplicating this code this much? 
struct AriaKnob820MinMax : AriaKnob820 {
	Darius *module;

	AriaKnob820MinMax(Darius* module) {
		this->module = module;
		AriaKnob820();
	}

	void onDragMove(const event::DragMove& e) override {
		module->lcdMode = MINMAX_MODE;
		module->lcdLastInteraction = 0.f;
		module->lcdStatus.lcdDirty = true;
		AriaKnob820::onDragMove(e);
	}
};

struct AriaKnob820Scale : AriaKnob820 {
	Darius *module;

	AriaKnob820Scale(Darius* module) {
		this->module = module;
		snap = true;
		AriaKnob820();
	}

	void onDragMove(const event::DragMove& e) override {
		module->lcdMode = SCALE_MODE;
		module->lcdLastInteraction = 0.f;
		module->lcdStatus.lcdDirty = true;
		AriaKnob820::onDragMove(e);
	}
};

struct AriaKnob820Slide : AriaKnob820 {
	Darius *module;

	AriaKnob820Slide(Darius* module) {
		this->module = module;
		AriaKnob820();
	}

	void onDragMove(const event::DragMove& e) override {
		module->lcdMode = SLIDE_MODE;
		module->lcdLastInteraction = 0.f;
		module->lcdStatus.lcdDirty = true;
		AriaKnob820::onDragMove(e);
	}
};

struct AriaRockerSwitchHorizontal800ModeReset : AriaRockerSwitchHorizontal800 {
	Darius *module;

	AriaRockerSwitchHorizontal800ModeReset(Darius* module) {
		this->module = module;
		AriaRockerSwitchHorizontal800();
	}

	void onDragStart(const event::DragStart& e) override {
		module->lcdMode = DEFAULT_MODE;
		module->lcdLastInteraction = 0.f;
		module->lcdStatus.lcdDirty = true;
		AriaRockerSwitchHorizontal800::onDragStart(e);
	}
};

// Also records the last one changed
template <class TParamWidget>
TParamWidget* createMainParam(math::Vec pos, Darius* module, int paramId, int lastChanged) {
	TParamWidget* o = new TParamWidget(module, lastChanged);
	o->box.pos = pos;
	if (module) {
		o->paramQuantity = module->paramQuantities[paramId];
	}
	return o;
}

struct AriaKnob820Route : AriaKnob820 {
	Darius *module;
	int lastChanged;

	AriaKnob820Route(Darius* module, int lastChanged) {
		this->module = module;
		this->lastChanged = lastChanged;
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/knob-820-arrow.svg")));
		minAngle = 0.25 * M_PI;
		maxAngle = 0.75 * M_PI;
		AriaKnob820();
	}

	void onDragMove(const event::DragMove& e) override {
		module->lcdMode = ROUTE_MODE;
		module->lcdLastInteraction = 0.f;
		module->lcdStatus.lcdDirty = true;
		module->lastRouteChanged = lastChanged;
		AriaKnob820::onDragMove(e);
	}
};

struct AriaKnob820TransparentCV : AriaKnob820Transparent {
	Darius *module;
	int lastChanged;

	AriaKnob820TransparentCV(Darius* module, int lastChanged) {
		this->module = module;
		this->lastChanged = lastChanged;
		AriaKnob820Transparent();
	}

	void onDragMove(const event::DragMove& e) override {
		module->lcdMode = KNOB_MODE;
		module->lcdLastInteraction = 0.f;
		module->lcdStatus.lcdDirty = true;
		module->lastCvChanged = lastChanged;
		AriaKnob820Transparent::onDragMove(e);
	}
};

struct DariusWidget : ModuleWidget {
	DariusWidget(Darius* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Darius.svg")));
		
		// Signature
		addChild(createWidget<AriaSignature>(mm2px(Vec(120.0, 114.538))));
		
		// Screws
		addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// The main area - lights, knobs and trigger outputs.
		for (int i = 0; i < 1; i++) {
			addChild(createLight<AriaInputLight>(              mm2px(Vec( 4.5, (16.0 + (6.5 * 7) + i * 13.0))), module, Darius::CV_LIGHT +    i));
			addParam(createMainParam<AriaKnob820TransparentCV>(mm2px(Vec( 4.5, (16.0 + (6.5 * 7) + i * 13.0))), module, Darius::CV_PARAM +    i, i));
			addParam(createMainParam<AriaKnob820Route>(        mm2px(Vec(14.5, (16.0 + (6.5 * 7) + i * 13.0))), module, Darius::ROUTE_PARAM + i, i));
			addChild(createLight<AriaOutputLight>(             mm2px(Vec( 9.5, (22.5 + (6.5 * 7) + i * 13.0))), module, Darius::GATE_LIGHT +  i));
			addOutput(createOutput<AriaJackTransparent>(       mm2px(Vec( 9.5, (22.5 + (6.5 * 7) + i * 13.0))), module, Darius::GATE_OUTPUT + i));
		}
		for (int i = 0; i < 2; i++) {
			addChild(createLight<AriaInputLight>(              mm2px(Vec(24.5, (16.0 + (6.5 * 6) + i * 13.0))), module, Darius::CV_LIGHT +    i + STEP2START));
			addParam(createMainParam<AriaKnob820TransparentCV>(mm2px(Vec(24.5, (16.0 + (6.5 * 6) + i * 13.0))), module, Darius::CV_PARAM +    i + STEP2START, i + STEP2START));
			addParam(createMainParam<AriaKnob820Route>(        mm2px(Vec(34.5, (16.0 + (6.5 * 6) + i * 13.0))), module, Darius::ROUTE_PARAM + i + STEP2START, i + STEP2START));
			addChild(createLight<AriaOutputLight>(             mm2px(Vec(29.5, (22.5 + (6.5 * 6) + i * 13.0))), module, Darius::GATE_LIGHT +  i + STEP2START));
			addOutput(createOutput<AriaJackTransparent>(       mm2px(Vec(29.5, (22.5 + (6.5 * 6) + i * 13.0))), module, Darius::GATE_OUTPUT + i + STEP2START));
		}
		for (int i = 0; i < 3; i++) {
			addChild(createLight<AriaInputLight>(              mm2px(Vec(44.5, (16.0 + (6.5 * 5) + i * 13.0))), module, Darius::CV_LIGHT +    i + STEP3START));
			addParam(createMainParam<AriaKnob820TransparentCV>(mm2px(Vec(44.5, (16.0 + (6.5 * 5) + i * 13.0))), module, Darius::CV_PARAM +    i + STEP3START, i + STEP3START));
			addParam(createMainParam<AriaKnob820Route>(        mm2px(Vec(54.5, (16.0 + (6.5 * 5) + i * 13.0))), module, Darius::ROUTE_PARAM + i + STEP3START, i + STEP3START));
			addChild(createLight<AriaOutputLight>(             mm2px(Vec(49.5, (22.5 + (6.5 * 5) + i * 13.0))), module, Darius::GATE_LIGHT +  i + STEP3START));
			addOutput(createOutput<AriaJackTransparent>(       mm2px(Vec(49.5, (22.5 + (6.5 * 5) + i * 13.0))), module, Darius::GATE_OUTPUT + i + STEP3START));
		}
		for (int i = 0; i < 4; i++) {
			addChild(createLight<AriaInputLight>(              mm2px(Vec(64.5, (16.0 + (6.5 * 4) + i * 13.0))), module, Darius::CV_LIGHT +    i + STEP4START));
			addParam(createMainParam<AriaKnob820TransparentCV>(mm2px(Vec(64.5, (16.0 + (6.5 * 4) + i * 13.0))), module, Darius::CV_PARAM +    i + STEP4START, i + STEP4START));
			addParam(createMainParam<AriaKnob820Route>(        mm2px(Vec(74.5, (16.0 + (6.5 * 4) + i * 13.0))), module, Darius::ROUTE_PARAM + i + STEP4START, i + STEP4START));
			addChild(createLight<AriaOutputLight>(             mm2px(Vec(69.5, (22.5 + (6.5 * 4) + i * 13.0))), module, Darius::GATE_LIGHT +  i + STEP4START));
			addOutput(createOutput<AriaJackTransparent>(       mm2px(Vec(69.5, (22.5 + (6.5 * 4) + i * 13.0))), module, Darius::GATE_OUTPUT + i + STEP4START));
		}
		for (int i = 0; i < 5; i++) {
			addChild(createLight<AriaInputLight>(              mm2px(Vec(84.5, (16.0 + (6.5 * 3) + i * 13.0))), module, Darius::CV_LIGHT +    i + STEP5START));
			addParam(createMainParam<AriaKnob820TransparentCV>(mm2px(Vec(84.5, (16.0 + (6.5 * 3) + i * 13.0))), module, Darius::CV_PARAM +    i + STEP5START, i + STEP5START));
			addParam(createMainParam<AriaKnob820Route>(        mm2px(Vec(94.5, (16.0 + (6.5 * 3) + i * 13.0))), module, Darius::ROUTE_PARAM + i + STEP5START, i + STEP5START));
			addChild(createLight<AriaOutputLight>(             mm2px(Vec(89.5, (22.5 + (6.5 * 3) + i * 13.0))), module, Darius::GATE_LIGHT +  i + STEP5START));
			addOutput(createOutput<AriaJackTransparent>(       mm2px(Vec(89.5, (22.5 + (6.5 * 3) + i * 13.0))), module, Darius::GATE_OUTPUT + i + STEP5START));
		}
		for (int i = 0; i < 6; i++) {
			addChild(createLight<AriaInputLight>(              mm2px(Vec(104.5, (16.0 + (6.5 * 2) + i * 13.0))), module, Darius::CV_LIGHT +    i + STEP6START));
			addParam(createMainParam<AriaKnob820TransparentCV>(mm2px(Vec(104.5, (16.0 + (6.5 * 2) + i * 13.0))), module, Darius::CV_PARAM +    i + STEP6START, i + STEP6START));
			addParam(createMainParam<AriaKnob820Route>(        mm2px(Vec(114.5, (16.0 + (6.5 * 2) + i * 13.0))), module, Darius::ROUTE_PARAM + i + STEP6START, i + STEP6START));
			addChild(createLight<AriaOutputLight>(             mm2px(Vec(109.5, (22.5 + (6.5 * 2) + i * 13.0))), module, Darius::GATE_LIGHT +  i + STEP6START));
			addOutput(createOutput<AriaJackTransparent>(       mm2px(Vec(109.5, (22.5 + (6.5 * 2) + i * 13.0))), module, Darius::GATE_OUTPUT + i + STEP6START));
		}
		for (int i = 0; i < 7; i++) {
			addChild(createLight<AriaInputLight>(              mm2px(Vec(124.5, (16.0 + (6.5 * 1) + i * 13.0))), module, Darius::CV_LIGHT +    i + STEP7START));
			addParam(createMainParam<AriaKnob820TransparentCV>(mm2px(Vec(124.5, (16.0 + (6.5 * 1) + i * 13.0))), module, Darius::CV_PARAM +    i + STEP7START, i + STEP7START));
			addParam(createMainParam<AriaKnob820Route>(        mm2px(Vec(134.5, (16.0 + (6.5 * 1) + i * 13.0))), module, Darius::ROUTE_PARAM + i + STEP7START, i + STEP7START));
			addChild(createLight<AriaOutputLight>(             mm2px(Vec(129.5, (22.5 + (6.5 * 1) + i * 13.0))), module, Darius::GATE_LIGHT +  i + STEP7START));
			addOutput(createOutput<AriaJackTransparent>(       mm2px(Vec(129.5, (22.5 + (6.5 * 1) + i * 13.0))), module, Darius::GATE_OUTPUT + i + STEP7START));
		}
		for (int i = 0; i < 8; i++) {
			addChild(createLight<AriaInputLight>(              mm2px(Vec(144.5, (16.0 + (6.5 * 0) + i * 13.0))), module, Darius::CV_LIGHT +    i + STEP8START));
			addParam(createMainParam<AriaKnob820TransparentCV>(mm2px(Vec(144.5, (16.0 + (6.5 * 0) + i * 13.0))), module, Darius::CV_PARAM +    i + STEP8START, i + STEP8START));
			addChild(createLight<AriaOutputLight>(             mm2px(Vec(149.5, (22.5 + (6.5 * 0) + i * 13.0))), module, Darius::GATE_LIGHT +  i + STEP8START));
			addOutput(createOutput<AriaJackTransparent>(       mm2px(Vec(149.5, (22.5 + (6.5 * 0) + i * 13.0))), module, Darius::GATE_OUTPUT + i + STEP8START));
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
		
		// Step count & First step
		addParam(createParam<AriaKnob820Snap>(mm2px(Vec(44.5, 22.5)), module, Darius::STEPFIRST_PARAM));
		addParam(createParam<AriaKnob820Snap>(mm2px(Vec(54.5, 22.5)), module, Darius::STEPCOUNT_PARAM));
		
		// Randomize
		addParam(createParam<AriaPushButton820Momentary>(mm2px(Vec(64.5, 22.5)), module, Darius::RANDCV_PARAM));
		addParam(createParam<AriaPushButton820Momentary>(mm2px(Vec(74.5, 22.5)), module, Darius::RANDROUTE_PARAM));
		
		// Seed
		addParam(createParam<AriaRockerSwitchVertical800>(mm2px(Vec(103.0, 112.0)), module, Darius::SEED_MODE_PARAM));
		addInput(createInput<AriaJackIn>(mm2px(Vec(109.5, 112.0)), module, Darius::SEED_INPUT));
		addChild(createLightCentered<SmallLight<InputLight>>(mm2px(Vec(108.7, 121.4)), module, Darius::SEED_LIGHT));

		// Output area //////////////////

		// Lcd
		Lcd::LcdFramebufferWidget<Darius> *lfb = new Lcd::LcdFramebufferWidget<Darius>(module);
		Lcd::LcdDrawWidget<Darius> *ldw = new Lcd::LcdDrawWidget<Darius>(module);
		lfb->box.pos = mm2px(Vec(10.3, 106.7));
		lfb->addChild(ldw);
		addChild(lfb);

		// Quantizer toggle
		addParam(createLcdParam<AriaRockerSwitchHorizontal800ModeReset>(mm2px(Vec(11.1, 99.7)), module, Darius::QUANTIZE_TOGGLE_PARAM));

		// Voltage Range
		addParam(createParam<AriaRockerSwitchHorizontal800Flipped>(mm2px(Vec(28.0, 118.8)), module, Darius::RANGE_PARAM));

		// Min & Max
		addParam(createLcdParam<AriaKnob820MinMax>(mm2px(Vec(49.5,  99.0)), module, Darius::MIN_PARAM));
		addParam(createLcdParam<AriaKnob820MinMax>(mm2px(Vec(49.5, 112.0)), module, Darius::MAX_PARAM));

		// Quantizer Key & Scale
		addParam(createLcdParam<AriaKnob820Scale>(mm2px(Vec(59.5, 99.0)), module, Darius::SCALE_PARAM));
		addParam(createLcdParam<AriaKnob820Scale>(mm2px(Vec(59.5, 112.0)), module, Darius::KEY_PARAM));

		// External Scale
		addInput(createInput<AriaJackIn>(mm2px(Vec(69.5, 99.0)), module, Darius::EXT_SCALE_INPUT));

		// Slide
		addParam(createLcdParam<AriaKnob820Slide>(mm2px(Vec(69.5, 112.0)), module, Darius::SLIDE_PARAM));

		// Output!
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(79.5, 112.0)), module, Darius::GLOBAL_GATE_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(89.5, 112.0)), module, Darius::CV_OUTPUT));
	}


	struct ResetCVItem : MenuItem {
		Darius *module;
		void onAction(const event::Action &e) override {
			module->resetCV = true;
		}
	};

	struct ResetRoutesItem : MenuItem {
		Darius *module;
		void onAction(const event::Action &e) override {
			module->resetRoutes = true;
		}
	};

	struct RoutesToTopItem : MenuItem {
		Darius *module;
		void onAction(const event::Action &e) override {
			module->routesToTop = true;
		}
	};

	struct RoutesToBottomItem : MenuItem {
		Darius *module;
		void onAction(const event::Action &e) override {
			module->routesToBottom = true;
		}
	};

	struct RoutesToEqualProbabilityItem : MenuItem {
		Darius *module;
		void onAction(const event::Action &e) override {
			module->routesToEqualProbability = true;
		}
	};

	struct RoutesToBinaryTreeItem : MenuItem {
		Darius *module;
		void onAction(const event::Action &e) override {
			module->routesToBinaryTree = true;
		}
	};

	void appendContextMenu(ui::Menu *menu) override {	
		Darius *module = dynamic_cast<Darius*>(this->module);
		assert(module);

		menu->addChild(new MenuSeparator());
		
		ResetCVItem *resetCVItem = createMenuItem<ResetCVItem>("Reset CV");
		resetCVItem->module = module;
		menu->addChild(resetCVItem);

		menu->addChild(new MenuSeparator());

		ResetRoutesItem *resetRoutesItem = createMenuItem<ResetRoutesItem>("Reset Routes (normal distribution skewing to center)");
		resetRoutesItem->module = module;
		menu->addChild(resetRoutesItem);

		RoutesToTopItem *routesToTopItem = createMenuItem<RoutesToTopItem>("Routes all to Top");
		routesToTopItem->module = module;
		menu->addChild(routesToTopItem);

		RoutesToBottomItem *routesToBottomItem = createMenuItem<RoutesToBottomItem>("Routes all to Bottom");
		routesToBottomItem->module = module;
		menu->addChild(routesToBottomItem);

		RoutesToEqualProbabilityItem *routesToEqualProbability = createMenuItem<RoutesToEqualProbabilityItem>("Routes Spread out (equal probability)");
		routesToEqualProbability->module = module;
		menu->addChild(routesToEqualProbability);

		RoutesToBinaryTreeItem *routesToBinaryTree = createMenuItem<RoutesToBinaryTreeItem>("Routes to Binary tree (equal probability)");
		routesToBinaryTree->module = module;
		menu->addChild(routesToBinaryTree);

		menu->addChild(new MenuSeparator());
	}
};

Model* modelDarius = createModel<Darius, DariusWidget>("Darius");
