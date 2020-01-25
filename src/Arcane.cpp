#include "plugin.hpp"
#include "network.hpp"
#include "quantizer.hpp"
#include <ctime>
#include <thread>

/* TODO

DOWNLOAD: Done, works reliably, Github is rate-limited but users will never run into it.
PARSE JSON API: Works. Everything trnasformed into useful data structure. 
QUANTIZER: Works well. Good CPU usage (pay only for performance you use) but only half the performance the official module.
CLOCK: It sorta work, but no run/reset yet. It's in Test.cpp for now, it's ready to move in there. 
PATTERNS: With the clock done, it should be easy.
FACEPLATES: Illustration PNGs cleaned up, cropped, and aligned. Gotta auto-trace them,  make the faceplaates, and do the code to change them.
LCD: That one is scary. Let's see if I can get away with fonts first. 
HANDLE LOAD/SAVE/RESET GRACEFULLY: Let's see what breaks
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
	
	// Absurdly huge performance gain not to send values each tick. Will do that unless people yell it breaks something.
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

	
	void sendVoltage(const ProcessArgs& args) {
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

	
	Arcane() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}
	
	void process(const ProcessArgs& args) override {
		if (!jsonParsed and readJsonDivider.process()) {
			jsonParsed = readTodaysFortune();
		}
		if (jsonParsed) {
			if (refreshDivider.process()){
				sendVoltage(args);
			}
			for (int i = 0; i < inputs[QNT_INPUT].getChannels(); i++)
				outputs[QNT_OUTPUT].setVoltage(quantize(inputs[QNT_INPUT].getVoltage(i), scale), i);
			outputs[QNT_OUTPUT].setChannels(inputs[QNT_INPUT].getChannels());
		} else {
			// Pass input as-is if JSON not parsed yet.
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
		addChild(createWidget<AriaSignature>(mm2px(Vec(91.0, 114.5))));
		
		// Screws - bottom left is moved not to obscure the arcana.
		addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
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
		addParam(createParam<AriaPushButton500>(mm2px(Vec(x + 16.0, y + 36.0)), module, Arcane::RESET_PARAM));
		addParam(createParam<AriaPushButton500>(mm2px(Vec(x + 19.4, y + 39.4)), module, Arcane::RUN_PARAM));
		addInput(createInput<AriaJackIn>(   mm2px(Vec(x + 08.0, y + 36.0)), module, Arcane::RESET_INPUT));
		addInput(createInput<AriaJackIn>(   mm2px(Vec(x + 24.0, y + 36.0)), module, Arcane::RUN_INPUT));
		
		// BPM
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 32.0, y + 36.0)), module, Arcane::BPM_NUM_OUTPUT));
		
		// Pulse/Ramp
		addParam(createParam<AriaRockerSwitchVertical800>(mm2px(Vec(x - 6.0, y + 54.0)), module, Arcane::PULSE_RAMP_PARAM));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 00.0, y + 54.0)), module, Arcane::BPM_32_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 08.0, y + 54.0)), module, Arcane::BPM_16_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 16.0, y + 54.0)), module, Arcane::BPM_8_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 24.0, y + 54.0)), module, Arcane::BPM_4_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 32.0, y + 54.0)), module, Arcane::BPM_1_OUTPUT));
				
		// B C D E
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 00.0, y + 64.0)), module, Arcane::PATTERN_B_32_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 08.0, y + 64.0)), module, Arcane::PATTERN_B_16_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 16.0, y + 64.0)), module, Arcane::PATTERN_B_8_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 24.0, y + 64.0)), module, Arcane::PATTERN_B_4_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 32.0, y + 64.0)), module, Arcane::PATTERN_B_1_OUTPUT));
		
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 00.0, y + 72.0)), module, Arcane::PATTERN_C_32_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 08.0, y + 72.0)), module, Arcane::PATTERN_C_16_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 16.0, y + 72.0)), module, Arcane::PATTERN_C_8_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 24.0, y + 72.0)), module, Arcane::PATTERN_C_4_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 32.0, y + 72.0)), module, Arcane::PATTERN_C_1_OUTPUT));
		
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 00.0, y + 80.0)), module, Arcane::PATTERN_D_32_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 08.0, y + 80.0)), module, Arcane::PATTERN_D_16_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 16.0, y + 80.0)), module, Arcane::PATTERN_D_8_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 24.0, y + 80.0)), module, Arcane::PATTERN_D_4_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 32.0, y + 80.0)), module, Arcane::PATTERN_D_1_OUTPUT));
		
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 00.0, y + 88.0)), module, Arcane::PATTERN_E_32_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 08.0, y + 88.0)), module, Arcane::PATTERN_E_16_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 16.0, y + 88.0)), module, Arcane::PATTERN_E_8_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 24.0, y + 88.0)), module, Arcane::PATTERN_E_4_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 32.0, y + 88.0)), module, Arcane::PATTERN_E_1_OUTPUT));
		
		// Debug Output
		// addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(35.0, 119.0)), module, Arcane::DEBUG_1_OUTPUT));
		// addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(45.0, 119.0)), module, Arcane::DEBUG_2_OUTPUT));
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
		addChild(createWidget<AriaSignature>(mm2px(Vec(26.06, 114.5))));
		
		// Screws
		addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
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
		addParam(createParam<AriaPushButton500>(mm2px(Vec(x + 16.0, y + 36.0)), module, Arcane::RESET_PARAM));
		addParam(createParam<AriaPushButton500>(mm2px(Vec(x + 19.4, y + 39.4)), module, Arcane::RUN_PARAM));
		addInput(createInput<AriaJackIn>(   mm2px(Vec(x + 08.0, y + 36.0)), module, Arcane::RESET_INPUT));
		addInput(createInput<AriaJackIn>(   mm2px(Vec(x + 24.0, y + 36.0)), module, Arcane::RUN_INPUT));

		// BPM
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 32.0, y + 36.0)), module, Arcane::BPM_NUM_OUTPUT));
		
		// Pulse/Ramp
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 00.0, y + 54.0)), module, Arcane::BPM_32_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 08.0, y + 54.0)), module, Arcane::BPM_16_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 16.0, y + 54.0)), module, Arcane::BPM_8_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 24.0, y + 54.0)), module, Arcane::BPM_4_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 32.0, y + 54.0)), module, Arcane::BPM_1_OUTPUT));
				
		// B C D E
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 00.0, y + 64.0)), module, Arcane::PATTERN_B_32_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 08.0, y + 64.0)), module, Arcane::PATTERN_B_16_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 16.0, y + 64.0)), module, Arcane::PATTERN_B_8_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 24.0, y + 64.0)), module, Arcane::PATTERN_B_4_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 32.0, y + 64.0)), module, Arcane::PATTERN_B_1_OUTPUT));
		
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 00.0, y + 72.0)), module, Arcane::PATTERN_C_32_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 08.0, y + 72.0)), module, Arcane::PATTERN_C_16_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 16.0, y + 72.0)), module, Arcane::PATTERN_C_8_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 24.0, y + 72.0)), module, Arcane::PATTERN_C_4_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 32.0, y + 72.0)), module, Arcane::PATTERN_C_1_OUTPUT));
		
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 00.0, y + 80.0)), module, Arcane::PATTERN_D_32_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 08.0, y + 80.0)), module, Arcane::PATTERN_D_16_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 16.0, y + 80.0)), module, Arcane::PATTERN_D_8_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 24.0, y + 80.0)), module, Arcane::PATTERN_D_4_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 32.0, y + 80.0)), module, Arcane::PATTERN_D_1_OUTPUT));
		
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 00.0, y + 88.0)), module, Arcane::PATTERN_E_32_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 08.0, y + 88.0)), module, Arcane::PATTERN_E_16_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 16.0, y + 88.0)), module, Arcane::PATTERN_E_8_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 24.0, y + 88.0)), module, Arcane::PATTERN_E_4_OUTPUT));
		addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 32.0, y + 88.0)), module, Arcane::PATTERN_E_1_OUTPUT));
		
		// On Atout, the Pulse/Ramp rocker is at the bottom
		addParam(createParam<AriaRockerSwitchHorizontal800>(mm2px(Vec(7.0, y + 98.0)), module, Arcane::PULSE_RAMP_PARAM));

		
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
		// addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(5.0, 119.0)), module, Aleister::DEBUG_1_OUTPUT));
		// addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(15.0, 119.0)), module, Aleister::DEBUG_2_OUTPUT));
	}
};


Model* modelArcane   = createModel<Arcane, ArcaneWidget>("Arcane");
Model* modelAtout    = createModel<Arcane, AtoutWidget>("Atout");
Model* modelAleister = createModel<Aleister, AleisterWidget>("Aleister");