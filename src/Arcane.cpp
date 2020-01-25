#include "plugin.hpp"
#include "network.hpp"
#include "quantizer.hpp"
#include <ctime>
#include <thread>

/* TODO

DOWNLOAD: Done, works reliably, Github is rate-limited but users will never run into it.
PARSE JSON API: Works. Everything trnasformed into useful data structure. 
QUANTIZER: Works well. Good CPU usage (pay only for performance you use) but only half the performance the official module.
CLOCK: It works! Or so I think.
PATTERNS: They seem to work OK. 
FACEPLATES: Illustration PNGs cleaned up, cropped, and aligned. Gotta auto-trace them, make the faceplaates, and do the code to change them.
LCD: That one is scary. Let's see if I can get away with fonts first. 
HANDLE LOAD/SAVE/RESET GRACEFULLY: Let's just see what breaks.
EXPANDER SYNC: Prolly not that hard, it's just a kinda r/w socket thing I think.
SERVER: That one is the trivial part.

*/

// The singleton owner downloads the the fortune from the repository.
// Other modules look for the cached file.
// Long name to avoid shared namespace collisions.
static bool ariaSalvatriceArcaneSingletonOwned = false;

// The Fool having no number, Death having no name, and additive roman numerals
// being used are all intentional. Translations are chosen to match closely the
// Marseilles naming, including the unusual "House of God" name for the Tower.
const std::array<std::string, 22> arcanaNames = {{
	"The Fool",
	"I. The Magician",
	"II. The Popess",
	"III. The Empress",
	"IIII. The Emperor",
	"V. The Pope",
	"VI. The Lover",
	"VII. The Chariot",
	"VIII. Justice",
	"VIIII. The Hermit",
	"X. The Wheel of Fortune",
	"XI. Strength",
	"XII. The Hanged Man",
	"XIII.",
	"XIIII. Temperance",
	"XV. The Devil",
	"XVI. The House of God",
	"XVII. The Star",
	"XVIII. The Moon",
	"XVIIII. The Sun",
	"XX. Judgement",
	"XXI. The World"	
}};

// FIXME: It'd be better to move it within the struct, but I couldn't figure out how to make threading work if I do that.
void downloadTodaysFortune() {
	// Get the date in UTC
	char todayUtc[11];
	time_t localTime = time(0);
	localTime = localTime - 60 * 60 * 12; // Offset by -12 hours, fortunes are up at 12:00 AM UTC
	tm *utcTime = gmtime(&localTime);
	strftime(todayUtc, 11, "%Y-%m-%d", utcTime);
	// Craft the URL and the filename. The URL is rate-limited, but users should never run into it.
	// std::string url = "https://raw.githubusercontent.com/AriaSalvatrice/Arcane/master/v1/" + std::string(todayUtc) + ".json"; // < FIXME
	std::string url = "https://aria.dog/upload/2020/01/" + std::string(todayUtc) + ".json"; // < FIXME
	std::string filename = asset::user("AriaSalvatrice/Arcane/").c_str() + std::string(todayUtc) + ".json";
	// Request it the url and save it
	float progress = 0.f;
	network::requestDownload(url, filename, &progress);
}


// Shared functionality for Arcane, Atout and Aleister
struct ArcaneBase : Module {
	bool owningSingleton = false;
	bool jsonParsed = false;
	bool staticValuesSent = false;
	
	// These are read from JSON
	int arcana, bpm, wish;
	std::array<int, 8> notePattern;
	bool patternB[16], patternC[16], patternD[16], patternE[16], scale[12]; // There is no pattern A
	
	// FIXME - figure out how to use a timer instead
	dsp::ClockDivider readJsonDivider; 
	
	// Absurdly huge performance gain not to send all static values each tick. Will do that unless people yell it breaks something.
	dsp::ClockDivider refreshDivider; 


	bool readTodaysFortune() {		
		// Craft the filename
		char todayUtc[11];
		time_t localTime = time(0);
		localTime = localTime - 60 * 60 * 12; // Offset by -12 hours, fortunes are up at 12:00 AM UTC
		tm *utcTime = gmtime(&localTime);
		strftime(todayUtc, 11, "%Y-%m-%d", utcTime);
		std::string filename = asset::user("AriaSalvatrice/Arcane/").c_str() + std::string(todayUtc) + ".json";
		// Open the file
		FILE* jsonFile = fopen(filename.c_str(), "r");
		if (!jsonFile) {
			fclose(jsonFile);
			return false;
		}
		// Read the JSON
		json_error_t error;
		json_t* rootJ = json_loadf(jsonFile, 0, &error);
		if (!rootJ) {
			fclose(jsonFile);
			return false;
		}
		fclose(jsonFile);	
		// Parse the JSON
		json_t* arcanaJ = json_object_get(rootJ, "arcana");
		if (arcanaJ) arcana = json_integer_value(arcanaJ);
		
		int patternBnum = 0;
		json_t* patternBnumJ = json_object_get(rootJ, "patternB");
		if (patternBnumJ) patternBnum = json_integer_value(patternBnumJ);
		for (int i = 0; i < 16; ++i)
			patternB[15 - i] = (patternBnum >> i) & 1;
		
		int patternCnum = 0;
		json_t* patternCnumJ = json_object_get(rootJ, "patternC");
		if (patternCnumJ) patternCnum = json_integer_value(patternCnumJ);
		for (int i = 0; i < 16; ++i)
			patternC[15 - i] = (patternCnum >> i) & 1;
		
		int patternDnum = 0;
		json_t* patternDnumJ = json_object_get(rootJ, "patternD");
		if (patternDnumJ) patternDnum = json_integer_value(patternDnumJ);
		for (int i = 0; i < 16; ++i)
			patternD[15 - i] = (patternDnum >> i) & 1;
		
		int patternEnum = 0;
		json_t* patternEnumJ = json_object_get(rootJ, "patternE");
		if (patternEnumJ) patternEnum = json_integer_value(patternEnumJ);
		for (int i = 0; i < 16; ++i)
			patternE[15 - i] = (patternEnum >> i) & 1;
		
		int scaleNum = 0;
		json_t* scaleNumJ = json_object_get(rootJ, "scale");
		if (scaleNumJ) scaleNum = json_integer_value(scaleNumJ);
		for (int i = 0; i < 12; ++i)
			scale[11 - i] = (scaleNum >> i) & 1;
		
		json_t* notePatternJ = json_object_get(rootJ, "notePattern");		
		if (notePatternJ) {
			for (int i = 0; i < 8; i++) {
				json_t* noteJ = json_array_get(notePatternJ, i);
				if (noteJ)
					notePattern[i] = json_integer_value(noteJ);
			}
		}
		
		json_t* bpmJ = json_object_get(rootJ, "bpm");
		if (bpmJ) bpm = json_integer_value(bpmJ);
		
		json_t* wishJ = json_object_get(rootJ, "wish");
		if (wishJ) wish = json_integer_value(wishJ);
		
		return true;
	}

		
	~ArcaneBase() { 
		// On destruction, release the singleton. 
		if (owningSingleton) { 
			ariaSalvatriceArcaneSingletonOwned = false;
		}
	}

	
	ArcaneBase() {
		readJsonDivider.setDivision(100000);
		refreshDivider.setDivision(128);
				
		// First created claims the singleton
		if (! ariaSalvatriceArcaneSingletonOwned) {
			ariaSalvatriceArcaneSingletonOwned = true;
			owningSingleton = true;
		}
		// On first run, create the config directories. Does nothing on subsequent ones. 
		system::createDirectory(asset::user("AriaSalvatrice"));
		system::createDirectory(asset::user("AriaSalvatrice/Arcane"));
		// Check if we already have today's JSON and parse it to more useful types
		bool jsonParsed = readTodaysFortune();
		// Download in a background thread if we don't have a valid JSON file read. process() will read the file later.
		if (owningSingleton and !jsonParsed) {
			std::thread t(downloadTodaysFortune);
			t.detach();
		}
	}
};



// This controls both Arcane and Atout, as only their views differ.
struct Arcane : ArcaneBase {
	enum ParamIds {
		RUN_PARAM,
		RESET_PARAM,
		PULSE_RAMP_PARAM,
		PULSE_WIDTH_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		QNT_INPUT,
		RUN_INPUT,
		RESET_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		QNT_OUTPUT,
		SCALE_OUTPUT,
		SCALE_PADDED_OUTPUT,
		BPM_NUM_OUTPUT,
		BPM_32_OUTPUT,
		BPM_16_OUTPUT,
		BPM_8_OUTPUT,
		BPM_4_OUTPUT,
		BPM_1_OUTPUT,
		ARCANA_OUTPUT,
		PATTERN_B_32_OUTPUT, // There is no pattern A
		PATTERN_B_16_OUTPUT,
		PATTERN_B_8_OUTPUT,
		PATTERN_B_4_OUTPUT,
		PATTERN_B_1_OUTPUT,
		PATTERN_C_32_OUTPUT,
		PATTERN_C_16_OUTPUT,
		PATTERN_C_8_OUTPUT,
		PATTERN_C_4_OUTPUT,
		PATTERN_C_1_OUTPUT,
		PATTERN_D_32_OUTPUT,
		PATTERN_D_16_OUTPUT,
		PATTERN_D_8_OUTPUT,
		PATTERN_D_4_OUTPUT,
		PATTERN_D_1_OUTPUT,
		PATTERN_E_32_OUTPUT,
		PATTERN_E_16_OUTPUT,
		PATTERN_E_8_OUTPUT,
		PATTERN_E_4_OUTPUT,
		PATTERN_E_1_OUTPUT,
		DEBUG_1_OUTPUT,
		DEBUG_2_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	// Clock. Aleister doesn't need to keep track of this so it goes here.
	int phaseCounter = 0;
	float phase = 0.f;
	dsp::PulseGenerator pulseThirtySecondGenerator, pulseSixteenthGenerator, pulseEighthGenerator, pulseQuarterGenerator, pulseBarGenerator;
	bool pulseThirtySecond = false, pulseSixteenth = false, pulseEighth = false, pulseQuarter = false, pulseBar = false;
	int thirtySecondCounter = 0, sixteenthCounter = 0, eighthCounter = 0, quarterCounter = 0, quarterInBarCounter = 0, barCounter = 0;
	float thirtySecondPulseWidth = 0.f, sixteenthPulseWidth = 0.f, eighthPulseWidth = 0.f, quarterPulseWidth = 0.f, barPulseWidth = 0.f;
	bool running = true;
	
	dsp::SchmittTrigger runCvTrigger;
	dsp::SchmittTrigger resetCvTrigger;
	dsp::SchmittTrigger resetButtonTrigger;
	
	
	void sendStaticVoltage(const ProcessArgs& args) {
		outputs[ARCANA_OUTPUT].setVoltage( arcana * 0.1f );
		outputs[BPM_NUM_OUTPUT].setVoltage (log2f(1.0f / (120.f / bpm)));
		
		int notesInScale = 0;
		for (int i = 0; i < 12; i++)
			if (scale[i]) notesInScale++;
		outputs[DEBUG_2_OUTPUT].setVoltage( notesInScale );
		for (int i = 0; i < 8; i++) { // FIXME - not converted to voltage!
			outputs[SCALE_OUTPUT].setVoltage( (notePattern[i] / 12.f), i);
			float paddedOutput = i < notesInScale ? (notePattern[i] / 12.f) : (notePattern[i] / 12.f + 1.f);
			outputs[SCALE_PADDED_OUTPUT].setVoltage(paddedOutput, i);
		}
		outputs[SCALE_OUTPUT].setChannels(notesInScale);
		outputs[SCALE_PADDED_OUTPUT].setChannels(8);
		staticValuesSent = true;
	}
	
	
	void processReset(const ProcessArgs& args){
		if (resetCvTrigger.process(inputs[RESET_INPUT].getVoltage()) or resetButtonTrigger.process(params[RESET_PARAM].getValue())){
			phase = 0.f;
			phaseCounter = 0;
			thirtySecondCounter = 0;
			sixteenthCounter = 0;
			eighthCounter = 0;
			quarterCounter = 0;
			quarterInBarCounter = 0;
			barCounter = 0;
			pulseThirtySecond = false;
			pulseSixteenth = false;
			pulseEighth = false;
			pulseQuarter = false;
			pulseBar = false;
		}
	}
	
	void processRunStatus(const ProcessArgs& args){
		if (runCvTrigger.process(inputs[RUN_INPUT].getVoltage())){
			running = !running;
			params[RUN_PARAM].setValue(running);
		}
		running = params[RUN_PARAM].getValue();
	}


	// I have no idea whatsoever how a clock is supposed to be implemented btw.
	void updateClock(const ProcessArgs& args) {

		thirtySecondPulseWidth =  60.f / bpm * params[PULSE_WIDTH_PARAM].getValue() / 100.f / 8;
		sixteenthPulseWidth = 60.f / bpm * params[PULSE_WIDTH_PARAM].getValue() / 100.f / 4;
		eighthPulseWidth = 60.f / bpm * params[PULSE_WIDTH_PARAM].getValue() / 100.f / 2;
		quarterPulseWidth = 60.f / bpm * params[PULSE_WIDTH_PARAM].getValue() / 100.f;
		barPulseWidth = 60.f / bpm * params[PULSE_WIDTH_PARAM].getValue() / 100.f * 4;

		phase += bpm / 60.f * 512.f / args.sampleRate; // High resolution to get a smooth ramp
		if (phase >= 1.0) {
			phase -= 1.0;
			if (phaseCounter > 0 ) {
				if ( phaseCounter % 512 == 0 ) {
					pulseQuarterGenerator.trigger(quarterPulseWidth); 
					quarterCounter = ( quarterCounter == 15 ? 0 : quarterCounter + 1 ); // FIXME - CRASHES FOR A REASON!!
					if ( quarterInBarCounter == 3 ) {
						quarterInBarCounter = 0;
						barCounter = ( barCounter == 15 ? 0 : barCounter + 1 );
						pulseBarGenerator.trigger(barPulseWidth); 
					} else {
						quarterInBarCounter++;
					}
				}
				if ( phaseCounter % 256 == 0 ) {
					pulseEighthGenerator.trigger(eighthPulseWidth); 
					eighthCounter = ( eighthCounter == 15 ? 0 : eighthCounter + 1 );
				}
				if ( phaseCounter % 128 == 0 ) {
					pulseSixteenthGenerator.trigger(sixteenthPulseWidth); 
					sixteenthCounter = ( sixteenthCounter == 15 ? 0 : sixteenthCounter + 1 );
				}
				if ( phaseCounter % 64 == 0 ) {
					pulseThirtySecondGenerator.trigger(thirtySecondPulseWidth);
					thirtySecondCounter = ( thirtySecondCounter == 15 ? 0 : thirtySecondCounter + 1 );
				}
			}
			phaseCounter = ( phaseCounter == 512 ) ? 0 : phaseCounter + 1;
		}
		pulseBar = pulseBarGenerator.process(args.sampleTime);
		pulseQuarter = pulseQuarterGenerator.process(args.sampleTime);
		pulseEighth = pulseEighthGenerator.process(args.sampleTime);
		pulseSixteenth = pulseSixteenthGenerator.process(args.sampleTime);
		pulseThirtySecond = pulseThirtySecondGenerator.process(args.sampleTime);
	}

	void sendClock(const ProcessArgs& args) {
		if (params[PULSE_RAMP_PARAM].getValue()) { // Ramp
			outputs[BPM_1_OUTPUT].setVoltage(  ((phase + phaseCounter)      / 512.f * 2.5f) + quarterInBarCounter * 2.5f );
			outputs[BPM_4_OUTPUT].setVoltage(  (phase + phaseCounter)       / 512.f * 10.f );  
			outputs[BPM_8_OUTPUT].setVoltage(  (phase + phaseCounter % 256) / 256.f * 10.f );
			outputs[BPM_16_OUTPUT].setVoltage( (phase + phaseCounter % 128) / 128.f * 10.f );
			outputs[BPM_32_OUTPUT].setVoltage( (phase + phaseCounter % 64)  / 64.f  * 10.f ); 
		} else { // Pulse 
			outputs[BPM_1_OUTPUT].setVoltage(  pulseBar          ? 10.f : 0.f );
			outputs[BPM_4_OUTPUT].setVoltage(  pulseQuarter      ? 10.f : 0.f );
			outputs[BPM_8_OUTPUT].setVoltage(  pulseEighth       ? 10.f : 0.f ); 
			outputs[BPM_16_OUTPUT].setVoltage( pulseSixteenth    ? 10.f : 0.f );
			outputs[BPM_32_OUTPUT].setVoltage( pulseThirtySecond ? 10.f : 0.f );
		}
	}
	
	// Yeah I know, copy-paste cowgirl coding in here. But it works, punk. 
	// This is where the bulk of the CPU time goes. Can I improve it? I don't see how, seems unsafe to skip steps on a clock.
	void sendPatterns(const ProcessArgs& args) {
		outputs[PATTERN_B_32_OUTPUT].setVoltage( (pulseThirtySecond and patternB[thirtySecondCounter]) ? 10.f : 0.f );
		outputs[PATTERN_C_32_OUTPUT].setVoltage( (pulseThirtySecond and patternC[thirtySecondCounter]) ? 10.f : 0.f );
		outputs[PATTERN_D_32_OUTPUT].setVoltage( (pulseThirtySecond and patternD[thirtySecondCounter]) ? 10.f : 0.f );
		outputs[PATTERN_E_32_OUTPUT].setVoltage( (pulseThirtySecond and patternE[thirtySecondCounter]) ? 10.f : 0.f );
		
		outputs[PATTERN_B_16_OUTPUT].setVoltage( (pulseSixteenth    and patternB[sixteenthCounter])    ? 10.f : 0.f );
		outputs[PATTERN_C_16_OUTPUT].setVoltage( (pulseSixteenth    and patternC[sixteenthCounter])    ? 10.f : 0.f );
		outputs[PATTERN_D_16_OUTPUT].setVoltage( (pulseSixteenth    and patternD[sixteenthCounter])    ? 10.f : 0.f );
		outputs[PATTERN_E_16_OUTPUT].setVoltage( (pulseSixteenth    and patternE[sixteenthCounter])    ? 10.f : 0.f );
		
		outputs[PATTERN_B_8_OUTPUT].setVoltage(  (pulseEighth       and patternB[eighthCounter])       ? 10.f : 0.f );
		outputs[PATTERN_C_8_OUTPUT].setVoltage(  (pulseEighth       and patternC[eighthCounter])       ? 10.f : 0.f );
		outputs[PATTERN_D_8_OUTPUT].setVoltage(  (pulseEighth       and patternD[eighthCounter])       ? 10.f : 0.f );
		outputs[PATTERN_E_8_OUTPUT].setVoltage(  (pulseEighth       and patternE[eighthCounter])       ? 10.f : 0.f );
												 
		outputs[PATTERN_B_4_OUTPUT].setVoltage(  (pulseQuarter      and patternB[quarterCounter])      ? 10.f : 0.f );
		outputs[PATTERN_C_4_OUTPUT].setVoltage(  (pulseQuarter      and patternC[quarterCounter])      ? 10.f : 0.f );
		outputs[PATTERN_D_4_OUTPUT].setVoltage(  (pulseQuarter      and patternD[quarterCounter])      ? 10.f : 0.f );
		outputs[PATTERN_E_4_OUTPUT].setVoltage(  (pulseQuarter      and patternE[quarterCounter])      ? 10.f : 0.f );
												 
		outputs[PATTERN_B_1_OUTPUT].setVoltage(  (pulseBar          and patternB[barCounter])          ? 10.f : 0.f );
		outputs[PATTERN_C_1_OUTPUT].setVoltage(  (pulseBar          and patternC[barCounter])          ? 10.f : 0.f );
		outputs[PATTERN_D_1_OUTPUT].setVoltage(  (pulseBar          and patternD[barCounter])          ? 10.f : 0.f );
		outputs[PATTERN_E_1_OUTPUT].setVoltage(  (pulseBar          and patternE[barCounter])          ? 10.f : 0.f );		
	}
	
	Arcane() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(RUN_PARAM, 0.f, 1.f, 1.f, "Run");
		configParam(RESET_PARAM, 0.f, 1.f, 0.f, "Reset");
		configParam(PULSE_WIDTH_PARAM, 1.f, 99.f, 1.f, "Pulse Width");
	}
	
	void process(const ProcessArgs& args) override {
		if (!jsonParsed and readJsonDivider.process()) jsonParsed = readTodaysFortune();
		if (jsonParsed) {
			if (refreshDivider.process()) sendStaticVoltage(args);
			
			processReset(args);
			processRunStatus(args);
			if (running) updateClock(args);
			sendClock(args); // Send even if not running
			
			sendPatterns(args);
			
			// Quantize
			for (int i = 0; i < inputs[QNT_INPUT].getChannels(); i++)
				outputs[QNT_OUTPUT].setVoltage(quantize(inputs[QNT_INPUT].getVoltage(i), scale), i);
			outputs[QNT_OUTPUT].setChannels(inputs[QNT_INPUT].getChannels());
		} else { // JSON not parsed, pass quantizer input as-is.
			for (int i = 0; i < inputs[QNT_INPUT].getChannels(); i++)
				outputs[QNT_OUTPUT].setVoltage(inputs[QNT_INPUT].getVoltage(i), i);
			outputs[QNT_OUTPUT].setChannels(inputs[QNT_INPUT].getChannels());
		}
	}
};


// Aleister is an expander, but it also works stand-alone.
struct Aleister : ArcaneBase {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(PATTERN_B_OUTPUT, 16),
		ENUMS(PATTERN_C_OUTPUT, 16),
		ENUMS(PATTERN_D_OUTPUT, 16),
		ENUMS(PATTERN_E_OUTPUT, 16),
		DEBUG_1_OUTPUT,
		DEBUG_2_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		ENUMS(PATTERN_B_LIGHT, 16),
		ENUMS(PATTERN_C_LIGHT, 16),
		ENUMS(PATTERN_D_LIGHT, 16),
		ENUMS(PATTERN_E_LIGHT, 16),
		NUM_LIGHTS
	};
	
	void sendVoltage(const ProcessArgs& args) {
		for (int i = 0; i < 16; i++) {
			outputs[PATTERN_B_OUTPUT + i].setVoltage(patternB[i] ? 10.f : 0.f);
			outputs[PATTERN_C_OUTPUT + i].setVoltage(patternC[i] ? 10.f : 0.f);
			outputs[PATTERN_D_OUTPUT + i].setVoltage(patternD[i] ? 10.f : 0.f);
			outputs[PATTERN_E_OUTPUT + i].setVoltage(patternE[i] ? 10.f : 0.f);
		}
	}

	void processLights(const ProcessArgs& args) {
		for (int i = 0; i < 16; i++) {
			lights[PATTERN_B_LIGHT + i].setBrightness(patternB[i] ? 1.f : 0.f);
			lights[PATTERN_C_LIGHT + i].setBrightness(patternC[i] ? 1.f : 0.f);
			lights[PATTERN_D_LIGHT + i].setBrightness(patternD[i] ? 1.f : 0.f);
			lights[PATTERN_E_LIGHT + i].setBrightness(patternE[i] ? 1.f : 0.f);
		}
	}
	
	Aleister() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}
	
	void process(const ProcessArgs& args) override {
		if (!jsonParsed and readJsonDivider.process()) {
			jsonParsed = readTodaysFortune();
		}
		if (jsonParsed) {
			// sendVoltage(args);
			if (refreshDivider.process()){
				sendVoltage(args);
				processLights(args);
			}
		}
	}
};


struct ArcaneWidget : ModuleWidget {
	// Offset
	float x = 80.32;
	float y = 18.0;
		
	ArcaneWidget(Arcane* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Arcane.svg")));
		
		// Signature
		addChild(createWidget<AriaSignature>(mm2px(Vec(101.0, 114.5))));
		
		// Screws - I want to give the impression there's 3 x 2 screens, the arcana hiding the left ones.
		addChild(createWidget<AriaScrew>(Vec(box.size.x - 10 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<AriaScrew>(Vec(box.size.x - 10 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
			
		// Quantizer
		addInput(createInput<AriaJackIn>(   mm2px(Vec(x + 00.0, y + 00.0)), module, Arcane::QNT_INPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 32.0, y + 00.0)), module, Arcane::QNT_OUTPUT));
		
		// Scale notes
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 00.0, y + 08.0)), module, Arcane::SCALE_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 32.0, y + 08.0)), module, Arcane::SCALE_PADDED_OUTPUT));

		// Arcane
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 00.0, y + 36.0)), module, Arcane::ARCANA_OUTPUT));
		
		// Reset/Run inputs and jacks
		addParam(createParam<AriaPushButton500Momentary>(mm2px(Vec(x + 16.0, y + 36.0)), module, Arcane::RESET_PARAM));
		addParam(createParam<AriaPushButton500>(mm2px(Vec(x + 19.4, y + 39.4)), module, Arcane::RUN_PARAM));
		addInput(createInput<AriaJackIn>(   mm2px(Vec(x + 08.0, y + 36.0)), module, Arcane::RESET_INPUT));
		addInput(createInput<AriaJackIn>(   mm2px(Vec(x + 24.0, y + 36.0)), module, Arcane::RUN_INPUT));
		
		// BPM
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 32.0, y + 36.0)), module, Arcane::BPM_NUM_OUTPUT));
		
		// Pulse/Ramp
		addParam(createParam<AriaRockerSwitchVertical800>(mm2px(Vec(x - 6.0, y + 54.0)), module, Arcane::PULSE_RAMP_PARAM));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 00.0, y + 54.0)), module, Arcane::BPM_1_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 08.0, y + 54.0)), module, Arcane::BPM_4_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 16.0, y + 54.0)), module, Arcane::BPM_8_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 24.0, y + 54.0)), module, Arcane::BPM_16_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 32.0, y + 54.0)), module, Arcane::BPM_32_OUTPUT));
				
		// B C D E
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 00.0, y + 64.0)), module, Arcane::PATTERN_B_1_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 08.0, y + 64.0)), module, Arcane::PATTERN_B_4_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 16.0, y + 64.0)), module, Arcane::PATTERN_B_8_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 24.0, y + 64.0)), module, Arcane::PATTERN_B_16_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 32.0, y + 64.0)), module, Arcane::PATTERN_B_32_OUTPUT));
		
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 00.0, y + 72.0)), module, Arcane::PATTERN_C_1_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 08.0, y + 72.0)), module, Arcane::PATTERN_C_4_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 16.0, y + 72.0)), module, Arcane::PATTERN_C_8_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 24.0, y + 72.0)), module, Arcane::PATTERN_C_16_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 32.0, y + 72.0)), module, Arcane::PATTERN_C_32_OUTPUT));
		
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 00.0, y + 80.0)), module, Arcane::PATTERN_D_1_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 08.0, y + 80.0)), module, Arcane::PATTERN_D_4_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 16.0, y + 80.0)), module, Arcane::PATTERN_D_8_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 24.0, y + 80.0)), module, Arcane::PATTERN_D_16_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 32.0, y + 80.0)), module, Arcane::PATTERN_D_32_OUTPUT));
		
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 00.0, y + 88.0)), module, Arcane::PATTERN_E_1_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 08.0, y + 88.0)), module, Arcane::PATTERN_E_4_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 16.0, y + 88.0)), module, Arcane::PATTERN_E_8_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 24.0, y + 88.0)), module, Arcane::PATTERN_E_16_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 32.0, y + 88.0)), module, Arcane::PATTERN_E_32_OUTPUT));
		
		// Pulse width
		addParam(createParam<AriaKnob820>(mm2px(Vec(x + 3.8, y + 98.0)), module, Arcane::PULSE_WIDTH_PARAM));	
		
		// Debug Output
		addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(35.0, 119.0)), module, Arcane::DEBUG_1_OUTPUT));
		addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(45.0, 119.0)), module, Arcane::DEBUG_2_OUTPUT));
	}
};


// Atout is a smaller version of Arcane, otherwise identical.
struct AtoutWidget : ModuleWidget {
	// Offset
	float x = 3.2;
	float y = 18.0;
	
	AtoutWidget(Arcane* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Atout.svg")));
		
		// Signature
		addChild(createWidget<AriaSignature>(mm2px(Vec(31.06, 114.5))));
		
		// Screws
		addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		// addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<AriaScrew>(Vec(box.size.x - 5 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	
		// Quantizer
		addInput(createInput<AriaJackIn>(   mm2px(Vec(x + 00.0, y + 00.0)), module, Arcane::QNT_INPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 32.0, y + 00.0)), module, Arcane::QNT_OUTPUT));
		
		// Scale notes
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 00.0, y + 08.0)), module, Arcane::SCALE_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 32.0, y + 08.0)), module, Arcane::SCALE_PADDED_OUTPUT));

		// Arcane
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 00.0, y + 36.0)), module, Arcane::ARCANA_OUTPUT));

		// Reset/Run inputs and jacks
		addParam(createParam<AriaPushButton500Momentary>(mm2px(Vec(x + 16.0, y + 36.0)), module, Arcane::RESET_PARAM));
		addParam(createParam<AriaPushButton500>(mm2px(Vec(x + 19.4, y + 39.4)), module, Arcane::RUN_PARAM));
		addInput(createInput<AriaJackIn>(   mm2px(Vec(x + 08.0, y + 36.0)), module, Arcane::RESET_INPUT));
		addInput(createInput<AriaJackIn>(   mm2px(Vec(x + 24.0, y + 36.0)), module, Arcane::RUN_INPUT));

		// BPM
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 32.0, y + 36.0)), module, Arcane::BPM_NUM_OUTPUT));
		
		// Pulse/Ramp
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 00.0, y + 54.0)), module, Arcane::BPM_1_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 08.0, y + 54.0)), module, Arcane::BPM_4_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 16.0, y + 54.0)), module, Arcane::BPM_8_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 24.0, y + 54.0)), module, Arcane::BPM_16_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 32.0, y + 54.0)), module, Arcane::BPM_32_OUTPUT));
				
		// B C D E
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 00.0, y + 64.0)), module, Arcane::PATTERN_B_1_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 08.0, y + 64.0)), module, Arcane::PATTERN_B_4_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 16.0, y + 64.0)), module, Arcane::PATTERN_B_8_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 24.0, y + 64.0)), module, Arcane::PATTERN_B_16_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 32.0, y + 64.0)), module, Arcane::PATTERN_B_32_OUTPUT));
		
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 00.0, y + 72.0)), module, Arcane::PATTERN_C_1_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 08.0, y + 72.0)), module, Arcane::PATTERN_C_4_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 16.0, y + 72.0)), module, Arcane::PATTERN_C_8_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 24.0, y + 72.0)), module, Arcane::PATTERN_C_16_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 32.0, y + 72.0)), module, Arcane::PATTERN_C_32_OUTPUT));
		
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 00.0, y + 80.0)), module, Arcane::PATTERN_D_1_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 08.0, y + 80.0)), module, Arcane::PATTERN_D_4_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 16.0, y + 80.0)), module, Arcane::PATTERN_D_8_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 24.0, y + 80.0)), module, Arcane::PATTERN_D_16_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 32.0, y + 80.0)), module, Arcane::PATTERN_D_32_OUTPUT));
		
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 00.0, y + 88.0)), module, Arcane::PATTERN_E_1_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 08.0, y + 88.0)), module, Arcane::PATTERN_E_4_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 16.0, y + 88.0)), module, Arcane::PATTERN_E_8_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 24.0, y + 88.0)), module, Arcane::PATTERN_E_16_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 32.0, y + 88.0)), module, Arcane::PATTERN_E_32_OUTPUT));
		
		// Pulse width
		addParam(createParam<AriaKnob820>(mm2px(Vec(x + 3.8, y + 96.0)), module, Arcane::PULSE_WIDTH_PARAM));	
		
		// On Atout, the Pulse/Ramp rocker is at the bottom
		// FIXME - It has an ugly shadow!
		addParam(createParam<AriaRockerSwitchHorizontal800>(mm2px(Vec(x + 3.8, y + 105.5)), module, Arcane::PULSE_RAMP_PARAM));
		
		// Debug Output
		// addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(25.0, 119.0)), module, Arcane::DEBUG_1_OUTPUT));
		// addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(35.0, 119.0)), module, Arcane::DEBUG_2_OUTPUT));
	}
};


// Aleister expresses the four binary patterns as gates instead of rhythms.
struct AleisterWidget : ModuleWidget {
	
	AleisterWidget(Aleister* module) { // FIXME it's its own struct!
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Aleister.svg")));
		
		// Signature
		addChild(createWidget<AriaSignature>(mm2px(Vec(28.76, 114.5))));
		
		// Screws
		addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		
		// Patterns
		float startX = 3.2;
		float startY = 18.0;
		
		for (int i = 0; i < 8; i++) {
			addChild(createLight<AriaJackLight<OutputLight>>(mm2px(Vec(startX + 0.2 + (i * 8.0), startY + 00.2)), module, Aleister::PATTERN_B_LIGHT + i + 0));
			addChild(createLight<AriaJackLight<OutputLight>>(mm2px(Vec(startX + 0.2 + (i * 8.0), startY + 08.2)), module, Aleister::PATTERN_B_LIGHT + i + 8));
			addChild(createLight<AriaJackLight<OutputLight>>(mm2px(Vec(startX + 0.2 + (i * 8.0), startY + 24.2)), module, Aleister::PATTERN_C_LIGHT + i + 0));
			addChild(createLight<AriaJackLight<OutputLight>>(mm2px(Vec(startX + 0.2 + (i * 8.0), startY + 32.2)), module, Aleister::PATTERN_C_LIGHT + i + 8));
			addChild(createLight<AriaJackLight<OutputLight>>(mm2px(Vec(startX + 0.2 + (i * 8.0), startY + 48.2)), module, Aleister::PATTERN_D_LIGHT + i + 0));
			addChild(createLight<AriaJackLight<OutputLight>>(mm2px(Vec(startX + 0.2 + (i * 8.0), startY + 56.2)), module, Aleister::PATTERN_D_LIGHT + i + 8));
			addChild(createLight<AriaJackLight<OutputLight>>(mm2px(Vec(startX + 0.2 + (i * 8.0), startY + 72.2)), module, Aleister::PATTERN_E_LIGHT + i + 0));
			addChild(createLight<AriaJackLight<OutputLight>>(mm2px(Vec(startX + 0.2 + (i * 8.0), startY + 80.2)), module, Aleister::PATTERN_E_LIGHT + i + 8));
						
			addOutput(createOutput<AriaJackTransparent>(mm2px(Vec(startX + (i * 8.0), startY + 00.0)), module, Aleister::PATTERN_B_OUTPUT + i + 0));
			addOutput(createOutput<AriaJackTransparent>(mm2px(Vec(startX + (i * 8.0), startY + 08.0)), module, Aleister::PATTERN_B_OUTPUT + i + 8));
			addOutput(createOutput<AriaJackTransparent>(mm2px(Vec(startX + (i * 8.0), startY + 24.0)), module, Aleister::PATTERN_C_OUTPUT + i + 0));
			addOutput(createOutput<AriaJackTransparent>(mm2px(Vec(startX + (i * 8.0), startY + 32.0)), module, Aleister::PATTERN_C_OUTPUT + i + 8));
			addOutput(createOutput<AriaJackTransparent>(mm2px(Vec(startX + (i * 8.0), startY + 48.0)), module, Aleister::PATTERN_D_OUTPUT + i + 0));
			addOutput(createOutput<AriaJackTransparent>(mm2px(Vec(startX + (i * 8.0), startY + 56.0)), module, Aleister::PATTERN_D_OUTPUT + i + 8));
			addOutput(createOutput<AriaJackTransparent>(mm2px(Vec(startX + (i * 8.0), startY + 72.0)), module, Aleister::PATTERN_E_OUTPUT + i + 0));
			addOutput(createOutput<AriaJackTransparent>(mm2px(Vec(startX + (i * 8.0), startY + 80.0)), module, Aleister::PATTERN_E_OUTPUT + i + 8));
		}
		
		// Debug Output
		addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(5.0, 119.0)), module, Aleister::DEBUG_1_OUTPUT));
		addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(15.0, 119.0)), module, Aleister::DEBUG_2_OUTPUT));
	}
};


Model* modelArcane   = createModel<Arcane, ArcaneWidget>("Arcane");
Model* modelAtout    = createModel<Arcane, AtoutWidget>("Atout");
Model* modelAleister = createModel<Aleister, AleisterWidget>("Aleister");