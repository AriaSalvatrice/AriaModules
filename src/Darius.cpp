#include "plugin.hpp"
#include "prng.hpp"

// Warning - this module was created with very little C++ experience, and features were 
// added to it later without regard for code quality. This is maintained exploratory code, not good design.

const int STEP1START = 0;  //               00        
const int STEP2START = 1;  //             02  01            
const int STEP3START = 3;  //           05  04  03          
const int STEP4START = 6;  //         09  08  07  06        
const int STEP5START = 10; //       14  13  12  11  10      
const int STEP6START = 15; //     20  19  18  17  16  15    
const int STEP7START = 21; //   27  26  25  24  23  22  21  
const int STEP8START = 28; // 35  34  33  32  31  30  29  28
const int STEP9START = 36; // (Panel is rotated 90 degrees counter-clockwise compared to this diagram)


enum LCDModes {
	SCALE_MODE,
	NOTE_MODE,
	MINMAX_MODE
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
	bool resetCV = false;
	bool resetRoutes = false;
	bool pianoDisplay[12] = {true, false, false, false, false, false, false, true, true, false, false, false};
	bool lcdDirty = false;
	std::string lcdText = "HULLO!!HIYA";
	int stepFirst = 1;
	int stepLast = 8;
	int step = 0;
	int node = 0;
	int lastNode = 0;
	int lastGate = 0;
	int pathTraveled[8] = { 0, -1, -1, -1, -1, -1, -1, -1}; // -1 = not gone there yet
	int lcdMode = MINMAX_MODE;
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
		configParam(STEPFIRST_PARAM, 1.f, 8.f, 1.f, "First step");
		configParam(STEPCOUNT_PARAM, 1.f, 8.f, 8.f, "Last step");
		configParam(RANDCV_PARAM, 0.f, 1.f, 0.f, "Randomize CV knobs");
		configParam(RANDROUTE_PARAM, 0.f, 1.f, 0.f, "Meta-randomize random route knobs");
		configParam(SEED_MODE_PARAM, 0.f, 1.f, 0.f, "New random seed on first or all nodes");
		configParam(RANGE_PARAM, 0.f, 1.f, 0.f, "Voltage output range");
		configParam(MIN_PARAM, 0.f, 10.f, 0.f, "Minimum CV/Note");
		configParam(MAX_PARAM, 0.f, 10.f, 10.f, "Maximum CV/Note");
		configParam(KEY_PARAM, 0.f, 1.f, 0.f, "Key");
		configParam(SCALE_PARAM, 0.f, 1.f, 0.f, "Scale");
		configParam(SLIDE_PARAM, 0.f, 1.f, 0.f, "Slide");
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
		shSeedNextFirst = true;
		resetPathTraveled(args);
		for (int i = 0; i < 36; i++)
			outputs[GATE_OUTPUT + i].setVoltage(0.f);
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
		if (step >= stepLast || step < stepFirst - 1) {
			shSeedNextFirst = true;
			step = 0;
			node = 0;
			lastNode = 0;
			resetPathTraveled(args);
			lightsReset = true;
			for(int i = 0; i < stepFirst - 1; i++) {
				step++;
				nodeForward(args);
			}
		}
	}
	
	void nodeForward(const ProcessArgs& args){
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
	
	void nodeBack(const ProcessArgs& args){
		lightsReset = true;
		node = pathTraveled[step];
		// FIXME - This conditional avoids a bizarre problem where randomSeed goes NaN. Not sure what's exactly going on!!
		if (step < 7) pathTraveled[step + 1] = -1; 
		lastNode = node;
	}
	
	void sendGateOutput(const ProcessArgs& args){
		if (inputs[STEP_INPUT].isConnected()){
			outputs[GATE_OUTPUT + node].setVoltage(inputs[STEP_INPUT].getVoltage());
		} else {
			outputs[GATE_OUTPUT + lastGate].setVoltage(0.f);
			outputs[GATE_OUTPUT + node].setVoltage(10.f);
		}
	}
	
	// FIXME - I changed how Min/Max work!
	void setVoltageOutput(const ProcessArgs& args){
		if (params[RANGE_PARAM].getValue() == 0.f ) {
			outputs[CV_OUTPUT].setVoltage(params[CV_PARAM + node].getValue());
		} else {
			outputs[CV_OUTPUT].setVoltage(params[CV_PARAM + node].getValue() - 5.0);
		}
	}
	
	void updateLights(const ProcessArgs& args){
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
		lights[GATE_LIGHT].setBrightness( (stepFirst <= 1 && stepLast >= 1 ) ? 1.f : 0.f );
		for (int i = STEP2START; i < STEP3START; i++)
			lights[GATE_LIGHT + i].setBrightness( (stepFirst <= 2 && stepLast >= 2 ) ? 1.f : 0.f );
		for (int i = STEP3START; i < STEP4START; i++)
			lights[GATE_LIGHT + i].setBrightness( (stepFirst <= 3 && stepLast >= 3 ) ? 1.f : 0.f );
		for (int i = STEP4START; i < STEP5START; i++)
			lights[GATE_LIGHT + i].setBrightness( (stepFirst <= 4 && stepLast >= 4 ) ? 1.f : 0.f );
		for (int i = STEP5START; i < STEP6START; i++)
			lights[GATE_LIGHT + i].setBrightness( (stepFirst <= 5 && stepLast >= 5 ) ? 1.f : 0.f );
		for (int i = STEP6START; i < STEP7START; i++)
			lights[GATE_LIGHT + i].setBrightness( (stepFirst <= 6 && stepLast >= 6 ) ? 1.f : 0.f );
		for (int i = STEP7START; i < STEP8START; i++)
			lights[GATE_LIGHT + i].setBrightness( (stepFirst <= 7 && stepLast >= 7 ) ? 1.f : 0.f );
		for (int i = STEP8START; i < STEP9START; i++){
			lights[GATE_LIGHT + i].setBrightness( (stepFirst <= 8 && stepLast >= 8 ) ? 1.f : 0.f );
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

		setRunStatus(args);
		setStepStatus(args);

		if (steppedForward)
			nodeForward(args);
		if (steppedBack)
			nodeBack(args);

		sendGateOutput(args);
		setVoltageOutput(args);
		updateLights(args);	
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


// Passes the module to the created knobs
template <class TParamWidget>
TParamWidget* createLCDParam(math::Vec pos, Darius* module, int paramId) {
	TParamWidget* o = new TParamWidget(module);
	o->box.pos = pos;
	if (module) {
		o->paramQuantity = module->paramQuantities[paramId];
	}
	return o;
}

struct AriaKnob820MinMax : AriaKnob820 {
	Darius *module;

	AriaKnob820MinMax(Darius* module) {
		this->module = module;
		AriaKnob820();
	}

	void onDragMove(const event::DragMove& e) override {
		module->lcdMode = SCALE_MODE;
		module->lcdDirty = true;
		AriaKnob820::onDragMove(e);
	}
};


// The draw widget from Arcane, adapted to Darius.
// I dunno yet how to abstract out the code better to avoid more copy-paste.
struct LCDDariusDrawWidget : TransparentWidget {
	Darius *module;
	std::array<std::shared_ptr<Svg>, 95> asciiSvg; // 32 to 126, the printable range
	std::array<std::shared_ptr<Svg>, 24> pianoSvg; // 0..11: Unlit, 12..23 = Lit

	LCDDariusDrawWidget(Darius *module) {
		this->module = module;
		if (module) {
			box.size = mm2px(Vec(36.0, 10.0));
			for (int i = 0; i < 12; i++) // Unlit
				pianoSvg[i] = APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/lcd/piano/u" + std::to_string(i) + ".svg"));
			for (int i = 0; i < 12; i++) // Lit
				pianoSvg[i + 12] = APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/lcd/piano/l" + std::to_string(i) + ".svg"));
			for (int i = 0; i < 95; i++)
				asciiSvg[i] = APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/lcd/Fixed_v01/" + std::to_string(i + 32) + ".svg"));
		}
	}

	void draw(const DrawArgs &args) override {
		if (module) {
			nvgScale(args.vg, 1.5, 1.5);
		
			// Piano if scale display
			if (module->lcdMode == SCALE_MODE) {
				nvgSave(args.vg);
				svgDraw(args.vg, pianoSvg[(module->pianoDisplay[0])  ? 12 :  0 ]->handle);
				nvgTranslate(args.vg, 6, 0);
				svgDraw(args.vg, pianoSvg[(module->pianoDisplay[1])  ? 13 :  1 ]->handle);
				nvgTranslate(args.vg, 5, 0);
				svgDraw(args.vg, pianoSvg[(module->pianoDisplay[2])  ? 14 :  2 ]->handle);
				nvgTranslate(args.vg, 5, 0);
				svgDraw(args.vg, pianoSvg[(module->pianoDisplay[3])  ? 15 :  3 ]->handle);
				nvgTranslate(args.vg, 5, 0);
				svgDraw(args.vg, pianoSvg[(module->pianoDisplay[4])  ? 16 :  4 ]->handle);
				nvgTranslate(args.vg, 7, 0);
				svgDraw(args.vg, pianoSvg[(module->pianoDisplay[5])  ? 17 :  5 ]->handle);
				nvgTranslate(args.vg, 6, 0);
				svgDraw(args.vg, pianoSvg[(module->pianoDisplay[6])  ? 18 :  6 ]->handle);
				nvgTranslate(args.vg, 5, 0);
				svgDraw(args.vg, pianoSvg[(module->pianoDisplay[7])  ? 19 :  7 ]->handle);
				nvgTranslate(args.vg, 5, 0);
				svgDraw(args.vg, pianoSvg[(module->pianoDisplay[8])  ? 20 :  8 ]->handle);
				nvgTranslate(args.vg, 5, 0);
				svgDraw(args.vg, pianoSvg[(module->pianoDisplay[9])  ? 21 :  9 ]->handle);
				nvgTranslate(args.vg, 5, 0);
				svgDraw(args.vg, pianoSvg[(module->pianoDisplay[10]) ? 22 : 10 ]->handle);
				nvgTranslate(args.vg, 5, 0);
				svgDraw(args.vg, pianoSvg[(module->pianoDisplay[11]) ? 23 : 11 ]->handle);
				nvgRestore(args.vg);
			}
		
			// 11 character display if scale display
			if (module->lcdMode == SCALE_MODE) {
				nvgSave(args.vg);
				nvgTranslate(args.vg, 0, 11);
				std::string lcdText = module->lcdText;
				lcdText.append(11, ' '); // Ensure the string is long enough
				for (int i = 0; i < 11; i++) {
					char c = lcdText.at(i);
					svgDraw(args.vg, asciiSvg[ c - 32 ]->handle);
					nvgTranslate(args.vg, 6, 0);
				}
				nvgRestore(args.vg);
			}
		}
	}
}; // LCDDariusDrawWidget



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
		
		// Step count & First step
		addParam(createParam<AriaKnob820Snap>(mm2px(Vec(44.5, 22.5)), module, Darius::STEPFIRST_PARAM));
		addParam(createParam<AriaKnob820Snap>(mm2px(Vec(54.5, 22.5)), module, Darius::STEPCOUNT_PARAM));
		
		// Randomize
		addParam(createParam<AriaPushButton820Momentary>(mm2px(Vec(64.5, 22.5)), module, Darius::RANDCV_PARAM));
		addParam(createParam<AriaPushButton820Momentary>(mm2px(Vec(74.5, 22.5)), module, Darius::RANDROUTE_PARAM));
		
		// Seed
		addParam(createParam<AriaRockerSwitchVertical800>(mm2px(Vec(103.0, 112.0)), module, Darius::SEED_MODE_PARAM));
		addInput(createInput<AriaJackIn>(mm2px(Vec(109.5, 112.0)), module, Darius::SEED_INPUT));

		// Output area //////////////////

		// LCD
		LCDFramebufferWidget<Darius> *lfb = new LCDFramebufferWidget<Darius>(module);
		LCDDariusDrawWidget *ldw = new LCDDariusDrawWidget(module);
		lfb->box.pos = mm2px(Vec(8.3, 106.7));
		lfb->addChild(ldw);
		addChild(lfb);

		// Quantizer toggle
		addParam(createParam<AriaRockerSwitchHorizontal800>(mm2px(Vec(9.1, 99.7)), module, Darius::QUANTIZE_TOGGLE_PARAM));

		// Voltage Range 
		addParam(createParam<AriaRockerSwitchHorizontal800>(mm2px(Vec(25.7, 118.7)), module, Darius::RANGE_PARAM));

		// Min & Max
		addParam(createLCDParam<AriaKnob820MinMax>(mm2px(Vec(49.5,  99.0)), module, Darius::MIN_PARAM));
		addParam(createLCDParam<AriaKnob820MinMax>(mm2px(Vec(49.5, 112.0)), module, Darius::MAX_PARAM));

		// Quantizer Key & Scale
		addParam(createParam<AriaKnob820>(mm2px(Vec(59.5, 99.0)), module, Darius::SCALE_PARAM));
		addParam(createParam<AriaKnob820>(mm2px(Vec(59.5, 112.0)), module, Darius::KEY_PARAM));

		// External Scale
		addInput(createInput<AriaJackIn>(mm2px(Vec(69.5, 99.0)), module, Darius::EXT_SCALE_INPUT));

		// Slide
		addParam(createParam<AriaKnob820>(mm2px(Vec(69.5, 112.0)), module, Darius::SLIDE_PARAM));

		// Output!
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(79.5, 112.0)), module, Darius::CV_OUTPUT));
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

	void appendContextMenu(ui::Menu *menu) override {	
		Darius *module = dynamic_cast<Darius*>(this->module);
		assert(module);

		menu->addChild(new MenuLabel());
		
		ResetCVItem *resetCVItem = createMenuItem<ResetCVItem>("Reset CV");
		resetCVItem->module = module;
		menu->addChild(resetCVItem);

		ResetRoutesItem *resetRoutesItem = createMenuItem<ResetRoutesItem>("Reset Routes");
		resetRoutesItem->module = module;
		menu->addChild(resetRoutesItem);
	}
};

Model* modelDarius = createModel<Darius, DariusWidget>("Darius");
