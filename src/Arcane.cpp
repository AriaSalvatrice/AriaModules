#include "plugin.hpp"
#include "network.hpp"
#include "quantizer.hpp"
#include "lcd.hpp"
#include <ctime>
#include <thread>

// This contains Arcane, Atout, and Aleister.

// The singleton owner downloads the the fortune from the repository.
// Other modules look for the cached file.
// Long name to avoid shared namespace collisions.
static bool ariaSalvatriceArcaneSingletonOwned = false;

// Fortunes are generated 10mn in advance to account for desync'd clocks. 
std::string getCurrentFortuneDate() {	
    char currentFortuneDate[11];
    time_t localTime = time(0);
    localTime = localTime - 60 * 60 * 12; // Offset by -12 hours, since fortunes are up at 12:00 AM UTC
    tm *utcTime = gmtime(&localTime);
    strftime(currentFortuneDate, 11, "%Y-%m-%d", utcTime);
    return currentFortuneDate;
}


// TODO: It'd be cleaner to move it within the struct, but I couldn't figure out how to make threading work if I do that.
void downloadTodaysFortune() {
    // Craft the URL and the filename. The URL is rate-limited, but users should never run into it.
    std::string url = "https://raw.githubusercontent.com/AriaSalvatrice/Arcane/master/v1/" + getCurrentFortuneDate() + ".json";
    std::string filename = asset::user("AriaSalvatrice/Arcane/").c_str() + getCurrentFortuneDate() + ".json";
    // Request it the url and save it
    float progress = 0.f;
    network::requestDownload(url, filename, &progress);
}


// Shared functionality for Arcane, Atout and Aleister
struct ArcaneBase : Module {
    bool owningSingleton = false;
    bool jsonParsed = false;
    
    // LCD stuff
    Lcd::LcdStatus lcdStatus;
    dsp::ClockDivider lcdDivider; 
    int lcdMode = 0;
    std::string todaysFortuneDate = getCurrentFortuneDate(); // Used to display on the LCD. Once set it changes only on reset.
    
    // These are read from JSON
    int arcana, bpm, wish;
    std::array<int, 8> notePattern;
    std::array<bool, 16> patternB, patternC, patternD, patternE; // There is no pattern A
    std::array<bool, 12> scale;
        
    dsp::ClockDivider readJsonDivider;
    // Huge performance gain not to send all static values each tick. Will do that unless people yell it breaks something.
    dsp::ClockDivider refreshDivider;	
    dsp::ClockDivider expanderDivider;

    bool readTodaysFortune() {		
        std::string filename = asset::user("AriaSalvatrice/Arcane/").c_str() + todaysFortuneDate + ".json";
        // Open the file
        FILE* jsonFile = fopen(filename.c_str(), "r");
        if (!jsonFile) return false;

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
        for (int i = 0; i < 12; ++i){
            scale[11 - i] = (scaleNum >> i) & 1;
            lcdStatus.pianoDisplay[11 - i] = (scaleNum >> i) & 1;
        }
        
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
    
    void onReset() override {
        todaysFortuneDate = getCurrentFortuneDate();
        jsonParsed = false;
        jsonParsed = readTodaysFortune();
        // On manual reset, we download if necessary, whether we own the singleton or not.
        if (!jsonParsed) {
            std::thread t(downloadTodaysFortune);
            t.detach();
        }
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
        
        // 64 lights aren't cheap, can we get away with this divider?
        // Seems fine @ 180 BPM 44Khz
        expanderDivider.setDivision(512);
                
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
}; // ArcaneBase



// This controls both Arcane and Atout, as only their views differ.
struct Arcane : ArcaneBase {
    enum ParamIds {
        RUN_PARAM,
        RESET_PARAM,
        PULSE_RAMP_PARAM,
        PULSE_WIDTH_PARAM, // 1.3.0
        NUM_PARAMS
    };
    enum InputIds {
        QNT_INPUT,
        RUN_INPUT,
        RESET_INPUT, // 1.3.0
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
        PATTERN_E_1_OUTPUT, // 1.3.0
        EXTERNAL_SCALE_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        EXPANDER_LIGHT, // 1.3.0
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
    
    // Only for Arcane
    bool cardDirty = true;
    int cardDelayCounter = 0;
        
    void sendStaticVoltage(const ProcessArgs& args) {
        outputs[ARCANA_OUTPUT].setVoltage( arcana * 0.1f );
        outputs[BPM_NUM_OUTPUT].setVoltage (log2f(1.0f / (120.f / bpm)));
        
        int notesInScale = 0;
        for (int i = 0; i < 12; i++)
            if (scale[i]) notesInScale++;
        for (int i = 0; i < 8; i++) {
            outputs[SCALE_OUTPUT].setVoltage( (notePattern[i] / 12.f), i);
            float paddedOutput = i < notesInScale ? (notePattern[i] / 12.f) : (notePattern[i] / 12.f + 1.f);
            outputs[SCALE_PADDED_OUTPUT].setVoltage(paddedOutput, i);
        }
        outputs[SCALE_OUTPUT].setChannels(notesInScale);
        outputs[SCALE_PADDED_OUTPUT].setChannels(8);
        for (int i = 0; i < 12; i++)
            outputs[EXTERNAL_SCALE_OUTPUT].setVoltage( (scale[i]) ? 10.f : 0.f, i);
        outputs[EXTERNAL_SCALE_OUTPUT].setChannels(12);
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
                    quarterCounter = ( quarterCounter == 15 ? 0 : quarterCounter + 1 );
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
    
    void processExpander(const ProcessArgs& args) {
        if (rightExpander.module and rightExpander.module->model == modelAleister) {
            lights[EXPANDER_LIGHT].setBrightness(1.f);
            
            int *message = (int*) rightExpander.module->leftExpander.producerMessage;			
            /*  Message structure
                0..3: 0 = disconnected, 1 = bar, 2 = 1/4, 3 = 1/8, 4 = 1/16, 5 = 1/32
                4..8: Bar, 1/4, 1/8, 1/16, 1/32 step
            */
            message[0] = 0;
            message[1] = 0;
            message[2] = 0;
            message[3] = 0;
            
            if ( outputs[PATTERN_B_1_OUTPUT].isConnected() )  message[0] = 1;
            if ( outputs[PATTERN_B_4_OUTPUT].isConnected() )  message[0] = 2;
            if ( outputs[PATTERN_B_8_OUTPUT].isConnected() )  message[0] = 3;
            if ( outputs[PATTERN_B_16_OUTPUT].isConnected() ) message[0] = 4;
            if ( outputs[PATTERN_B_32_OUTPUT].isConnected() ) message[0] = 5;

            if ( outputs[PATTERN_C_1_OUTPUT].isConnected() )  message[1] = 1;
            if ( outputs[PATTERN_C_4_OUTPUT].isConnected() )  message[1] = 2;
            if ( outputs[PATTERN_C_8_OUTPUT].isConnected() )  message[1] = 3;
            if ( outputs[PATTERN_C_16_OUTPUT].isConnected() ) message[1] = 4;
            if ( outputs[PATTERN_C_32_OUTPUT].isConnected() ) message[1] = 5;

            if ( outputs[PATTERN_D_1_OUTPUT].isConnected() )  message[2] = 1;
            if ( outputs[PATTERN_D_4_OUTPUT].isConnected() )  message[2] = 2;
            if ( outputs[PATTERN_D_8_OUTPUT].isConnected() )  message[2] = 3;
            if ( outputs[PATTERN_D_16_OUTPUT].isConnected() ) message[2] = 4;
            if ( outputs[PATTERN_D_32_OUTPUT].isConnected() ) message[2] = 5;

            if ( outputs[PATTERN_E_1_OUTPUT].isConnected() )  message[3] = 1;
            if ( outputs[PATTERN_E_4_OUTPUT].isConnected() )  message[3] = 2;
            if ( outputs[PATTERN_E_8_OUTPUT].isConnected() )  message[3] = 3;
            if ( outputs[PATTERN_E_16_OUTPUT].isConnected() ) message[3] = 4;
            if ( outputs[PATTERN_E_32_OUTPUT].isConnected() ) message[3] = 5;
            
            message[4] = barCounter;
            message[5] = quarterCounter;
            message[6] = eighthCounter;
            message[7] = sixteenthCounter;
            message[8] = thirtySecondCounter;
            
            // Flip messages at the end of the timestep
            rightExpander.module->leftExpander.messageFlipRequested = true;
            
        } else {
            lights[EXPANDER_LIGHT].setBrightness(0.f);
        }
    }
    
    // The screen is too small for descenders, and enlarging it would make things cramped, so labels are all uppercase.
    void processLcdText(const ProcessArgs& args) {
        lcdDivider.setDivision(args.sampleRate * 2); // 2 seconds. Any way to set it up from the constructor instead?
        if (jsonParsed) {
            switch (lcdMode) {
                case 0:
                    lcdStatus.lcdText2 = todaysFortuneDate;
                    lcdMode++;
                    break;
                case 1:
                    if (arcana == 0 ) lcdStatus.lcdText2 = "   FOOL    ";
                    if (arcana == 1 ) lcdStatus.lcdText2 = " MAGICIAN  ";
                    if (arcana == 2 ) lcdStatus.lcdText2 = "H.PRIESTESS";
                    if (arcana == 3 ) lcdStatus.lcdText2 = "  EMPRESS  ";
                    if (arcana == 4 ) lcdStatus.lcdText2 = "  EMPEROR  ";
                    if (arcana == 5 ) lcdStatus.lcdText2 = "HIEROPHANT ";
                    if (arcana == 6 ) lcdStatus.lcdText2 = "  LOVERS   ";
                    if (arcana == 7 ) lcdStatus.lcdText2 = "  CHARIOT  ";
                    if (arcana == 8 ) lcdStatus.lcdText2 = "  JUSTICE  ";
                    if (arcana == 9 ) lcdStatus.lcdText2 = "  HERMIT   ";
                    if (arcana == 10) lcdStatus.lcdText2 = "W. FORTUNE ";
                    if (arcana == 11) lcdStatus.lcdText2 = "  STRENGTH ";
                    if (arcana == 12) lcdStatus.lcdText2 = "HANGED MAN ";
                    if (arcana == 13) lcdStatus.lcdText2 = "           "; // Intentional
                    if (arcana == 14) lcdStatus.lcdText2 = "TEMPERANCE ";
                    if (arcana == 15) lcdStatus.lcdText2 = "   DEVIL   ";
                    if (arcana == 16) lcdStatus.lcdText2 = "   TOWER   ";
                    if (arcana == 17) lcdStatus.lcdText2 = "   STAR    ";
                    if (arcana == 18) lcdStatus.lcdText2 = "   MOON    ";
                    if (arcana == 19) lcdStatus.lcdText2 = "    SUN    ";
                    if (arcana == 20) lcdStatus.lcdText2 = " JUDGEMENT ";
                    if (arcana == 21) lcdStatus.lcdText2 = "   WORLD   ";
                    lcdMode++;
                    break;
                case 2:
                    lcdStatus.lcdText2 = "  " + std::to_string(bpm) + " BPM";
                    lcdMode++;
                    break;
                case 3:
                    if (wish == 0) lcdStatus.lcdText2 = "WISH:LUCK";
                    if (wish == 1) lcdStatus.lcdText2 = "WISH:LOVE";
                    if (wish == 2) lcdStatus.lcdText2 = "WISH:HEALTH";
                    if (wish == 3) lcdStatus.lcdText2 = "WISH:MONEY";
                    if (todaysFortuneDate != getCurrentFortuneDate()) {
                        lcdMode = 4;
                    } else {
                        lcdMode = 0;
                    }
                    break;
                case 4: 
                    lcdStatus.lcdText2 = "NEW ORACLE!";
                    lcdMode = 0;
                    break;
            }
        } else { // JSON not parsed
            lcdStatus.lcdText2 = (owningSingleton) ? "DOWNLOADING" : "WAIT ON D/L"; 
        }
    }
    
    void onReset() override {
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
        running = true;
        lcdMode = 0;
        cardDelayCounter = 0;
        lcdStatus.lcdDirty = true;
        cardDirty = true;
        ArcaneBase::onReset();
    }
    
    // Since there's no guarantee the patterns will be the same when the user
    // reloads the file, it makes no sense to save detailed status. All we
    // save is whether the clock is running or not. 
    json_t* dataToJson() override {
        json_t* rootJ = json_object();
        json_object_set_new(rootJ, "running", json_boolean(running));
        return rootJ;
    }
    
    void dataFromJson(json_t* rootJ) override {
        json_t* runningJ = json_object_get(rootJ, "running");
        if (runningJ) running = json_is_true(runningJ);
    }
    
    Arcane() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(RUN_PARAM, 0.f, 1.f, 1.f, "Run");
        configParam(RESET_PARAM, 0.f, 1.f, 0.f, "Reset");
        configParam(PULSE_WIDTH_PARAM, 1.f, 99.f, 1.f, "Pulse width for all outputs", "%");
        configParam(PULSE_RAMP_PARAM, 0.f, 1.f, 0.f, "Clock Pulse/Ramp output");
        lcdDivider.setDivision(1000); // Gets changed on first tick
        lcdStatus.lcdPage = Lcd::PIANO_AND_TEXT2_PAGE;
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
                outputs[QNT_OUTPUT].setVoltage(Quantizer::quantize(inputs[QNT_INPUT].getVoltage(i), scale), i);
            outputs[QNT_OUTPUT].setChannels(inputs[QNT_INPUT].getChannels());
        } else { // JSON not parsed, pass quantizer input as-is.
            for (int i = 0; i < inputs[QNT_INPUT].getChannels(); i++)
                outputs[QNT_OUTPUT].setVoltage(inputs[QNT_INPUT].getVoltage(i), i);
            outputs[QNT_OUTPUT].setChannels(inputs[QNT_INPUT].getChannels());
        }
        if (expanderDivider.process()) {
            processExpander(args);
        }
        
        if (lcdDivider.process()) {
            processLcdText(args);
            lcdStatus.lcdDirty = true;
            if (jsonParsed) {
                // Slow down loading the card by 8 secs, to simulate the user placing it manually themself
                // and give them time to read the message on the faceplate, conveying better the theme
                // of the module to new users.
                // When zooming in we lose the FB sometimes it seems, so a periodic refresh is in order
                cardDelayCounter = (cardDelayCounter == 4) ? 4 : cardDelayCounter + 1;
                cardDirty = true;
            }
        }
    }
}; // Arcane


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
        NUM_OUTPUTS
    };
    enum LightIds {
        ENUMS(PATTERN_B_LIGHT, 16),
        ENUMS(PATTERN_C_LIGHT, 16),
        ENUMS(PATTERN_D_LIGHT, 16),
        ENUMS(PATTERN_E_LIGHT, 16),
        ENUMS(PATTERN_B_STEP_LIGHT, 16),
        ENUMS(PATTERN_C_STEP_LIGHT, 16),
        ENUMS(PATTERN_D_STEP_LIGHT, 16),
        ENUMS(PATTERN_E_STEP_LIGHT, 16),
        EXPANDER_LIGHT,
        NUM_LIGHTS
    };
    
    int leftMessages[2][9] = {};
    
    void sendVoltage(const ProcessArgs& args) {
        // If the user connects only the first cable, assume they want it polyphonic
        bool polyBRequested = true, polyCRequested = true, polyDRequested = true, polyERequested = true;
        for (int i = 1; i < 16; i++) {
            if ( outputs[PATTERN_B_OUTPUT + i].isConnected() ) polyBRequested = false;
            if ( outputs[PATTERN_C_OUTPUT + i].isConnected() ) polyCRequested = false;
            if ( outputs[PATTERN_D_OUTPUT + i].isConnected() ) polyDRequested = false;
            if ( outputs[PATTERN_E_OUTPUT + i].isConnected() ) polyERequested = false;
        }

        // setChannels(16) throws warnings, but works normally. Ignoring the error to see only useful warnings.
        // https://github.com/VCVRack/Rack/issues/1524 - compiler bug
        if (polyBRequested) {
            for (int i = 0; i < 16; i++) outputs[PATTERN_B_OUTPUT].setVoltage(patternB[i] ? 10.f : 0.f, i);
            outputs[PATTERN_B_OUTPUT].setChannels(16);
        } else {
            for (int i = 0; i < 16; i++) outputs[PATTERN_B_OUTPUT + i].setVoltage(patternB[i] ? 10.f : 0.f);
            outputs[PATTERN_B_OUTPUT].setChannels(0);
        }
        if (polyCRequested) {
            for (int i = 0; i < 16; i++) outputs[PATTERN_C_OUTPUT].setVoltage(patternC[i] ? 10.f : 0.f, i);
            outputs[PATTERN_C_OUTPUT].setChannels(16);
        } else {
            for (int i = 0; i < 16; i++) outputs[PATTERN_C_OUTPUT + i].setVoltage(patternC[i] ? 10.f : 0.f);
            outputs[PATTERN_C_OUTPUT].setChannels(0);
        }
        if (polyDRequested) {
            for (int i = 0; i < 16; i++) outputs[PATTERN_D_OUTPUT].setVoltage(patternD[i] ? 10.f : 0.f, i);
            outputs[PATTERN_D_OUTPUT].setChannels(16);
        } else {
            for (int i = 0; i < 16; i++) outputs[PATTERN_D_OUTPUT + i].setVoltage(patternD[i] ? 10.f : 0.f);
            outputs[PATTERN_D_OUTPUT].setChannels(0);
        }
        if (polyERequested) {
            for (int i = 0; i < 16; i++) outputs[PATTERN_E_OUTPUT].setVoltage(patternE[i] ? 10.f : 0.f, i);
            outputs[PATTERN_E_OUTPUT].setChannels(16);
        } else {
            for (int i = 0; i < 16; i++) outputs[PATTERN_E_OUTPUT + i].setVoltage(patternE[i] ? 10.f : 0.f);
            outputs[PATTERN_E_OUTPUT].setChannels(0);
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

        leftExpander.producerMessage = leftMessages[0];
        leftExpander.consumerMessage = leftMessages[1];

    }
    
    // Turn on a step and turn off the others
    void turnOnPatternLight(int light, int step, const ProcessArgs& args) {
        for (int i = 0; i < 16; i++) {
            lights[light + i ].setBrightness( (i == step) ? 1.f : 0.f);
        }
    }
    
    void turnOffPatternLight(int light, const ProcessArgs& args) {
        for (int i = 0; i < 16; i++) {
            lights[light + i].setBrightness(0.f);
        }
    }
    
    void processExpander(const ProcessArgs& args) {
        if (leftExpander.module and ( leftExpander.module->model == modelArcane or leftExpander.module->model == modelAtout ) ) {
            lights[EXPANDER_LIGHT].setBrightness(1.f);
            int *message = (int*) leftExpander.consumerMessage;
            /*  Message structure
                0..3: 0 = disconnected, 1 = bar, 2 = 1/4, 3 = 1/8, 4 = 1/16, 5 = 1/32
                4..8: Bar, 1/4, 1/8, 1/16, 1/32 step
            */
            if (message[0]) {
                turnOnPatternLight(PATTERN_B_STEP_LIGHT, message[ message[0] + 3 ], args );
            } else {
                turnOffPatternLight(PATTERN_B_STEP_LIGHT, args );
            }
            if (message[1]) {
                turnOnPatternLight(PATTERN_C_STEP_LIGHT, message[ message[1] + 3 ], args );
            } else {
                turnOffPatternLight(PATTERN_C_STEP_LIGHT, args );
            }
            if (message[2]) {
                turnOnPatternLight(PATTERN_D_STEP_LIGHT, message[ message[2] + 3 ], args );
            } else {
                turnOffPatternLight(PATTERN_D_STEP_LIGHT, args );
            }
            if (message[3]) {
                turnOnPatternLight(PATTERN_E_STEP_LIGHT, message[ message[3] + 3 ], args );
            } else {
                turnOffPatternLight(PATTERN_E_STEP_LIGHT, args );
            }
        } else {
            lights[EXPANDER_LIGHT].setBrightness(0.f);
            turnOffPatternLight(PATTERN_B_STEP_LIGHT, args );
            turnOffPatternLight(PATTERN_C_STEP_LIGHT, args );
            turnOffPatternLight(PATTERN_D_STEP_LIGHT, args );
            turnOffPatternLight(PATTERN_E_STEP_LIGHT, args );
        }
    }
    
    void process(const ProcessArgs& args) override {
        if (!jsonParsed and readJsonDivider.process()) {
            jsonParsed = readTodaysFortune();
        }
        if (jsonParsed) {
            if (refreshDivider.process()){
                sendVoltage(args);
                processLights(args);
            }
        }
        
        if (expanderDivider.process()) {
            processExpander(args);
        }
    }
}; // Aleister


// The magnetic cards. You really feel the performance hit without a framebuffer here. 
struct CardFramebufferWidget : FramebufferWidget{
    Arcane *module;
    CardFramebufferWidget(Arcane *m){
        module = m;
    }

    void step() override{
        if (module) { // Required to avoid crashing module browser
            if(module->cardDirty){
                FramebufferWidget::dirty = true;
                module->cardDirty = false;
            }
            FramebufferWidget::step();
        }
    }
};

struct CardDrawWidget : TransparentWidget {
    Arcane *module;
    std::shared_ptr<Svg> cardSvg;
    
    CardDrawWidget(Arcane* module) {
        this->module = module;
        box.size = mm2px(Vec(69.5, 128.5));
    }
    
    void draw(const DrawArgs &args) override {
        if (module) {
            cardSvg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/Arcane/" + std::to_string(module->arcana) + ".svg"));
            if (module->cardDelayCounter == 4) svgDraw(args.vg, cardSvg->handle);
        }
    }
};


struct ArcaneWidget : ModuleWidget {
    // Offset
    float x = 80.32;
    float y = 18.0;
        
    ArcaneWidget(Arcane* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Arcane/Arcane.svg")));
        
        // Signature
        addChild(createWidget<AriaSignature>(mm2px(Vec(101.0, 114.5))));

        // Screws. First two, the leftmost ones, are hidden after the card loads.
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));	
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 10 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 10 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        
        // The card
        CardFramebufferWidget *cfb = new CardFramebufferWidget(module);
        CardDrawWidget *cdw = new CardDrawWidget(module);
        cfb->box.pos = mm2px(Vec(0.0, 0.0));
        cfb->addChild(cdw);
        addChild(cfb);
        
        // LCD
        Lcd::LcdFramebufferWidget<Arcane> *lfb = new Lcd::LcdFramebufferWidget<Arcane>(module);
        Lcd::LcdDrawWidget<Arcane> *ldw = new Lcd::LcdDrawWidget<Arcane>(module);
        lfb->box.pos = mm2px(Vec(83.6, 41.4));
        lfb->addChild(ldw);
        addChild(lfb);
                    
        // Quantizer
        addInput(createInput<AriaJackIn>(   mm2px(Vec(x + 00.0, y + 00.0)), module, Arcane::QNT_INPUT));
        addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 32.0, y + 00.0)), module, Arcane::QNT_OUTPUT));
        
        // Scale
        addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 00.0, y + 08.0)), module, Arcane::SCALE_OUTPUT));
        addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 16.0, y + 08.0)), module, Arcane::SCALE_PADDED_OUTPUT));
        addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 32.0, y + 08.0)), module, Arcane::EXTERNAL_SCALE_OUTPUT));

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
        
        // Expander light
        addChild(createLight<SmallLight<OutputLight>>(mm2px(Vec(x + 38.1, 125.2)), module, Arcane::EXPANDER_LIGHT));
    }
}; // ArcaneWidget



// Atout is a smaller version of Arcane, otherwise identical.
struct AtoutWidget : ModuleWidget {
    // Offset
    float x = 3.2;
    float y = 18.0;
    
    AtoutWidget(Arcane* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Arcane/Atout.svg")));
        
        // Signature
        addChild(createWidget<AriaSignature>(mm2px(Vec(31.06, 114.5))));
        
        // LCD	
        Lcd::LcdFramebufferWidget<Arcane> *fb = new Lcd::LcdFramebufferWidget<Arcane>(module);
        Lcd::LcdDrawWidget<Arcane> *ldw = new Lcd::LcdDrawWidget<Arcane>(module);
        fb->box.pos = mm2px(Vec(6.44, 41.4));
        fb->addChild(ldw);
        addChild(fb);
        
        // Screws
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 5 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    
        // Quantizer
        addInput(createInput<AriaJackIn>(   mm2px(Vec(x + 00.0, y + 00.0)), module, Arcane::QNT_INPUT));
        addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 32.0, y + 00.0)), module, Arcane::QNT_OUTPUT));
        
        // Scale
        addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 00.0, y + 08.0)), module, Arcane::SCALE_OUTPUT));
        addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 16.0, y + 08.0)), module, Arcane::SCALE_PADDED_OUTPUT));
        addOutput(createOutput<AriaJackOut>(mm2px(Vec(x + 32.0, y + 08.0)), module, Arcane::EXTERNAL_SCALE_OUTPUT));

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
        addParam(createParam<AriaRockerSwitchHorizontal800>(mm2px(Vec(x + 3.8, y + 105.5)), module, Arcane::PULSE_RAMP_PARAM));
        
        // Expander light
        addChild(createLight<SmallLight<OutputLight>>(mm2px(Vec(x + 39.0, 125.2)), module, Arcane::EXPANDER_LIGHT));
    }
}; // AtoutWidget




// Aleister expresses the four binary patterns as gates instead of rhythms.
struct AleisterWidget : ModuleWidget {
    
    // No BG, so I can overlay it on top of the normal light.
    // Would have preferred to go light blue, but yellow is the only color that feels readable but not jarring.
    struct AriaStepLight : AriaJackLight {
        AriaStepLight() {
            this->addBaseColor(nvgRGB(0xff, 0xcc, 0x03));
            this->bgColor = nvgRGBA(0xff, 0xff, 0xff, 0x00);
        }
    };
    
    AleisterWidget(Aleister* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Arcane/Aleister.svg")));
        
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
            addChild(createLight<AriaOutputLight>(mm2px(Vec(startX + (i * 8.0), startY + 00.f)), module, Aleister::PATTERN_B_LIGHT + i + 0));
            addChild(createLight<AriaOutputLight>(mm2px(Vec(startX + (i * 8.0), startY + 08.f)), module, Aleister::PATTERN_B_LIGHT + i + 8));
            addChild(createLight<AriaOutputLight>(mm2px(Vec(startX + (i * 8.0), startY + 24.f)), module, Aleister::PATTERN_C_LIGHT + i + 0));
            addChild(createLight<AriaOutputLight>(mm2px(Vec(startX + (i * 8.0), startY + 32.f)), module, Aleister::PATTERN_C_LIGHT + i + 8));
            addChild(createLight<AriaOutputLight>(mm2px(Vec(startX + (i * 8.0), startY + 48.f)), module, Aleister::PATTERN_D_LIGHT + i + 0));
            addChild(createLight<AriaOutputLight>(mm2px(Vec(startX + (i * 8.0), startY + 56.f)), module, Aleister::PATTERN_D_LIGHT + i + 8));
            addChild(createLight<AriaOutputLight>(mm2px(Vec(startX + (i * 8.0), startY + 72.f)), module, Aleister::PATTERN_E_LIGHT + i + 0));
            addChild(createLight<AriaOutputLight>(mm2px(Vec(startX + (i * 8.0), startY + 80.f)), module, Aleister::PATTERN_E_LIGHT + i + 8));
            
            addChild(createLight<AriaStepLight>(mm2px(Vec(startX + (i * 8.0), startY + 00.f)), module, Aleister::PATTERN_B_STEP_LIGHT + i + 0));
            addChild(createLight<AriaStepLight>(mm2px(Vec(startX + (i * 8.0), startY + 08.f)), module, Aleister::PATTERN_B_STEP_LIGHT + i + 8));
            addChild(createLight<AriaStepLight>(mm2px(Vec(startX + (i * 8.0), startY + 24.f)), module, Aleister::PATTERN_C_STEP_LIGHT + i + 0));
            addChild(createLight<AriaStepLight>(mm2px(Vec(startX + (i * 8.0), startY + 32.f)), module, Aleister::PATTERN_C_STEP_LIGHT + i + 8));
            addChild(createLight<AriaStepLight>(mm2px(Vec(startX + (i * 8.0), startY + 48.f)), module, Aleister::PATTERN_D_STEP_LIGHT + i + 0));
            addChild(createLight<AriaStepLight>(mm2px(Vec(startX + (i * 8.0), startY + 56.f)), module, Aleister::PATTERN_D_STEP_LIGHT + i + 8));
            addChild(createLight<AriaStepLight>(mm2px(Vec(startX + (i * 8.0), startY + 72.f)), module, Aleister::PATTERN_E_STEP_LIGHT + i + 0));
            addChild(createLight<AriaStepLight>(mm2px(Vec(startX + (i * 8.0), startY + 80.f)), module, Aleister::PATTERN_E_STEP_LIGHT + i + 8));
                        
            addOutput(createOutput<AriaJackTransparent>(mm2px(Vec(startX + (i * 8.0), startY + 00.0)), module, Aleister::PATTERN_B_OUTPUT + i + 0));
            addOutput(createOutput<AriaJackTransparent>(mm2px(Vec(startX + (i * 8.0), startY + 08.0)), module, Aleister::PATTERN_B_OUTPUT + i + 8));
            addOutput(createOutput<AriaJackTransparent>(mm2px(Vec(startX + (i * 8.0), startY + 24.0)), module, Aleister::PATTERN_C_OUTPUT + i + 0));
            addOutput(createOutput<AriaJackTransparent>(mm2px(Vec(startX + (i * 8.0), startY + 32.0)), module, Aleister::PATTERN_C_OUTPUT + i + 8));
            addOutput(createOutput<AriaJackTransparent>(mm2px(Vec(startX + (i * 8.0), startY + 48.0)), module, Aleister::PATTERN_D_OUTPUT + i + 0));
            addOutput(createOutput<AriaJackTransparent>(mm2px(Vec(startX + (i * 8.0), startY + 56.0)), module, Aleister::PATTERN_D_OUTPUT + i + 8));
            addOutput(createOutput<AriaJackTransparent>(mm2px(Vec(startX + (i * 8.0), startY + 72.0)), module, Aleister::PATTERN_E_OUTPUT + i + 0));
            addOutput(createOutput<AriaJackTransparent>(mm2px(Vec(startX + (i * 8.0), startY + 80.0)), module, Aleister::PATTERN_E_OUTPUT + i + 8));
        }
        
        // Expander light
        addChild(createLight<SmallLight<InputLight>>(mm2px(Vec(1.4, 125.2)), module, Aleister::EXPANDER_LIGHT));		
    }
}; // AleisterWidget


Model* modelArcane   = createModel<Arcane, ArcaneWidget>("Arcane");
Model* modelAtout    = createModel<Arcane, AtoutWidget>("Atout");
Model* modelAleister = createModel<Aleister, AleisterWidget>("Aleister");
