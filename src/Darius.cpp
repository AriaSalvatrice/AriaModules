/*  Copyright (C) 2019-2020 Aria Salvatrice
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/


// Warning - this module was created with very little C++ experience, and features were 
// added to it later without regard for code quality. This is maintained exploratory code, not good design.
// I do not know why it works.
// In fact it doesn't really work, if you process data at audio rates everything breaks.
// Every time I have to look at this code again, I add another comment here to discourage you from working on this code.


// Note: the module calls it a "path" internally, but they are called "routes" for the user.


#include "plugin.hpp"
#include "prng.hpp"
#include "quantizer.hpp"
#include "lcd.hpp"
#include "portablesequence.hpp"

namespace Darius {

const int STEP1START = 0;  //               00        
const int STEP2START = 1;  //             02  01            
const int STEP3START = 3;  //           05  04  03          
const int STEP4START = 6;  //         09  08  07  06        
const int STEP5START = 10; //       14  13  12  11  10      
const int STEP6START = 15; //     20  19  18  17  16  15    
const int STEP7START = 21; //   27  26  25  24  23  22  21  
const int STEP8START = 28; // 35  34  33  32  31  30  29  28
const int STEP9START = 36; // (Panel is rotated 90 degrees counter-clockwise compared to this diagram)

const int STEP_STARTS[9] = {STEP1START, STEP2START, STEP3START, STEP4START, STEP5START, STEP6START, STEP7START, STEP8START, STEP9START};

const int DISPLAYDIVIDER = 512;
const int PROCESSDIVIDER = 32;

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
    bool copyPortableSequence = false;
    bool pastePortableSequence = false;
    std::array<bool, 12> scale;
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
    float resetDelay = -1.f; // 0 when reset started
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
    dsp::ClockDivider processDivider;
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
        configParam(MIN_PARAM, 0.f, 10.f, 3.f, "Minimum CV/Note");
        configParam(MAX_PARAM, 0.f, 10.f, 5.f, "Maximum CV/Note");
        configParam(QUANTIZE_TOGGLE_PARAM, 0.f, 1.f, 1.f, "Precise CV/Quantized V/Oct");
        configParam(KEY_PARAM, 0.f, 11.f, 0.f, "Key");
        configParam(SCALE_PARAM, 0.f, (float) Quantizer::NUM_SCALES - 1, 2.f, "Scale");
        configParam(SLIDE_PARAM, 0.f, 10.f, 0.f, "Slide");
        for (size_t i = 0; i < 36; i++)
            configParam(CV_PARAM + i, 0.f, 10.f, 5.f, "CV");
        for (size_t i = 0; i < 36; i++)
            configParam(ROUTE_PARAM + i, 0.f, 1.f, 0.5f, "Random route");
        displayDivider.setDivision(DISPLAYDIVIDER);
        processDivider.setDivision(PROCESSDIVIDER);
        lcdStatus.layout = Lcd::TEXT1_AND_TEXT2_LAYOUT;
        lcdStatus.text1 = "MEDITATE..."; // Loading message
        lcdStatus.text2 = "MEDITATION."; // https://www.youtube.com/watch?v=JqLNY1zyQ6o
        for (size_t i = 0; i < 100; i++) random::uniform(); // The first few seeds we get seem bad, need more warming up. Might just be superstition.
    }
    
    json_t* dataToJson() override {
        json_t *rootJ = json_object();
        json_object_set_new(rootJ, "step", json_integer(step));
        json_object_set_new(rootJ, "node", json_integer(node));
        json_object_set_new(rootJ, "lastNode", json_integer(lastNode));
        json_object_set_new(rootJ, "lastGate", json_integer(lastGate));
        json_t *pathTraveledJ = json_array();
        for (size_t i = 0; i < 8; i++) {
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
            for (size_t i = 0; i < 8; i++) {
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
    // https://github.com/AriaSalvatrice/AriaModules/issues/14
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
                for (size_t i = 0; i < 36; i++) module->params[param + i].setValue(this->oldValues[i]);
            }
        }

        void redo() override {
            Darius *module = dynamic_cast<Darius*>(APP->engine->getModule(this->moduleId));
            if (module) {
                for (size_t i = 0; i < 36; i++) module->params[param + i].setValue(this->newValues[i]);
            }
        }
    };

    void randomizeCv(){
        std::array<float, 36> oldValues;
        std::array<float, 36> newValues;
        for (size_t i = 0; i < 36; i++) oldValues[i] = params[CV_PARAM + i].getValue();
        for (size_t i = 0; i < 36; i++) params[CV_PARAM + i].setValue(random::uniform() * 10.f);
        for (size_t i = 0; i < 36; i++) newValues[i] = params[CV_PARAM + i].getValue();
        APP->history->push(new BulkCvAction(this->id, "randomize Darius CV", CV_PARAM, oldValues, newValues));
    }
    
    void randomizeRoute(){
        std::array<float, 36> oldValues;
        std::array<float, 36> newValues;
        for (size_t i = 0; i < 36; i++) oldValues[i] = params[ROUTE_PARAM + i].getValue();
        for (size_t i = 0; i < 36; i++) params[ROUTE_PARAM + i].setValue(random::uniform());	
        for (size_t i = 0; i < 36; i++) newValues[i] = params[ROUTE_PARAM + i].getValue();
        APP->history->push(new BulkCvAction(this->id, "randomize Darius Routes", ROUTE_PARAM, oldValues, newValues));
    }
    
    void processResetCV(){
        resetCV = false;
        std::array<float, 36> oldValues;
        std::array<float, 36> newValues;
        for (size_t i = 0; i < 36; i++) oldValues[i] = params[CV_PARAM + i].getValue();
        for (size_t i = 0; i < 36; i++) params[CV_PARAM + i].setValue(5.f);	
        for (size_t i = 0; i < 36; i++) newValues[i] = params[CV_PARAM + i].getValue();
        APP->history->push(new BulkCvAction(this->id, "reset Darius CV", CV_PARAM, oldValues, newValues));
    }

    void processResetRoutes(){
        resetRoutes = false;
        std::array<float, 36> oldValues;
        std::array<float, 36> newValues;
        for (size_t i = 0; i < 36; i++) oldValues[i] = params[ROUTE_PARAM + i].getValue();
        for (size_t i = 0; i < 36; i++) params[ROUTE_PARAM + i].setValue(0.5f);	
        for (size_t i = 0; i < 36; i++) newValues[i] = params[ROUTE_PARAM + i].getValue();
        APP->history->push(new BulkCvAction(this->id, "reset Darius Routes", ROUTE_PARAM, oldValues, newValues));
    }

    void processRoutesToTop(){
        routesToTop = false;
        std::array<float, 36> oldValues;
        std::array<float, 36> newValues;
        for (size_t i = 0; i < 36; i++) oldValues[i] = params[ROUTE_PARAM + i].getValue();
        for (size_t i = 0; i < 36; i++) params[ROUTE_PARAM + i].setValue(0.f);	
        for (size_t i = 0; i < 36; i++) newValues[i] = params[ROUTE_PARAM + i].getValue();
        APP->history->push(new BulkCvAction(this->id, "set Darius Routes to Top", ROUTE_PARAM, oldValues, newValues));
    }

    void processRoutesToBottom(){
        routesToBottom = false;
        std::array<float, 36> oldValues;
        std::array<float, 36> newValues;
        for (size_t i = 0; i < 36; i++) oldValues[i] = params[ROUTE_PARAM + i].getValue();
        for (size_t i = 0; i < 36; i++) params[ROUTE_PARAM + i].setValue(1.f);	
        for (size_t i = 0; i < 36; i++) newValues[i] = params[ROUTE_PARAM + i].getValue();
        APP->history->push(new BulkCvAction(this->id, "set Darius Routes to Bottom", ROUTE_PARAM, oldValues, newValues));
    }

    void processRoutesToEqualProbability(){
        routesToEqualProbability = false;
        std::array<float, 36> oldValues;
        std::array<float, 36> newValues;
        for (size_t i = 0; i < 36; i++) oldValues[i] = params[ROUTE_PARAM + i].getValue();
        params[ROUTE_PARAM].setValue(0.5f);
        for (size_t i = 0; i < 2; i++) params[ROUTE_PARAM + i + STEP2START].setValue( (i + 1) / 3.f );
        for (size_t i = 0; i < 3; i++) params[ROUTE_PARAM + i + STEP3START].setValue( (i + 1) / 4.f );
        for (size_t i = 0; i < 4; i++) params[ROUTE_PARAM + i + STEP4START].setValue( (i + 1) / 5.f );
        for (size_t i = 0; i < 5; i++) params[ROUTE_PARAM + i + STEP5START].setValue( (i + 1) / 6.f );
        for (size_t i = 0; i < 6; i++) params[ROUTE_PARAM + i + STEP6START].setValue( (i + 1) / 7.f );
        for (size_t i = 0; i < 7; i++) params[ROUTE_PARAM + i + STEP7START].setValue( (i + 1) / 8.f );
        for (size_t i = 0; i < 36; i++) newValues[i] = params[ROUTE_PARAM + i].getValue();
        APP->history->push(new BulkCvAction(this->id, "set Darius Routes to Spread out", ROUTE_PARAM, oldValues, newValues));
    }

    // Thanks to stoermelder for the idea!
    // https://community.vcvrack.com/t/arias-cool-and-nice-thread-of-barely-working-betas-and-bug-squashing-darius-update/8208/13?u=aria_salvatrice
    void processRoutesToBinaryTree(){
        routesToBinaryTree = false;
        std::array<float, 36> oldValues;
        std::array<float, 36> newValues;
        for (size_t i = 0; i < 36; i++) oldValues[i] = params[ROUTE_PARAM + i].getValue();
        for (size_t i = 0; i < 36; i++) params[ROUTE_PARAM + i].setValue(0.5f);
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
        for (size_t i = 0; i < 36; i++) newValues[i] = params[ROUTE_PARAM + i].getValue();
        APP->history->push(new BulkCvAction(this->id, "set Darius Routes to Binary tree", ROUTE_PARAM, oldValues, newValues));
    }

    void importPortableSequence(){
        pastePortableSequence = false;
        std::array<float, 36> oldValues;
        std::array<float, 36> newValues;
        PortableSequence::Sequence sequence;
        sequence.fromClipboard();
        sequence.sort();
        sequence.clampValues();
        for (size_t i = 0; i < 36; i++) oldValues[i] = params[CV_PARAM + i].getValue(); 
        for(int i = 0; i < 1; i++) params[CV_PARAM + i + STEP1START].setValue( (sequence.notes.size() > 0) ? clamp(sequence.notes[0].pitch + 4.f, 0.f, 10.f) : 5.f);
        for(int i = 0; i < 2; i++) params[CV_PARAM + i + STEP2START].setValue( (sequence.notes.size() > 1) ? clamp(sequence.notes[1].pitch + 4.f, 0.f, 10.f) : 5.f);
        for(int i = 0; i < 3; i++) params[CV_PARAM + i + STEP3START].setValue( (sequence.notes.size() > 2) ? clamp(sequence.notes[2].pitch + 4.f, 0.f, 10.f) : 5.f);
        for(int i = 0; i < 4; i++) params[CV_PARAM + i + STEP4START].setValue( (sequence.notes.size() > 3) ? clamp(sequence.notes[3].pitch + 4.f, 0.f, 10.f) : 5.f);
        for(int i = 0; i < 5; i++) params[CV_PARAM + i + STEP5START].setValue( (sequence.notes.size() > 4) ? clamp(sequence.notes[4].pitch + 4.f, 0.f, 10.f) : 5.f);
        for(int i = 0; i < 6; i++) params[CV_PARAM + i + STEP6START].setValue( (sequence.notes.size() > 5) ? clamp(sequence.notes[5].pitch + 4.f, 0.f, 10.f) : 5.f);
        for(int i = 0; i < 7; i++) params[CV_PARAM + i + STEP7START].setValue( (sequence.notes.size() > 6) ? clamp(sequence.notes[6].pitch + 4.f, 0.f, 10.f) : 5.f);
        for(int i = 0; i < 8; i++) params[CV_PARAM + i + STEP8START].setValue( (sequence.notes.size() > 7) ? clamp(sequence.notes[7].pitch + 4.f, 0.f, 10.f) : 5.f);
        for (size_t i = 0; i < 36; i++) newValues[i] = params[CV_PARAM + i].getValue();
        APP->history->push(new BulkCvAction(this->id, "import Portable Sequence", CV_PARAM, oldValues, newValues));
    }

    // OK, this one is gonna be messy lol
    void exportPortableSequence(){
        copyPortableSequence = false;
        PortableSequence::Sequence sequence;
        PortableSequence::Note note;
        prng::prng localPrng;
        float localSeed;
        int currentNode = 0;

        if (inputs[SEED_INPUT].isConnected() and (inputs[SEED_INPUT].getVoltage() != 0.f) ) {
            localSeed = inputs[SEED_INPUT].getVoltage();
        } else {
            localSeed = random::uniform();
        }
        localPrng.init(localSeed, localSeed);

        note.length = 1.f;
        for (int i = 0; i < 8; i++) {
            note.start = (float) i;
            note.pitch = params[CV_PARAM + currentNode].getValue();
            if (params[QUANTIZE_TOGGLE_PARAM].getValue() == 1.f) {
                note.pitch = rescale(note.pitch, 0.f, 10.f, params[MIN_PARAM].getValue() - 4.f, params[MAX_PARAM].getValue() - 4.f);
                note.pitch = Quantizer::quantize(note.pitch, scale);
            } else {
                note.pitch = rescale(note.pitch, 0.f, 10.f, params[MIN_PARAM].getValue(), params[MAX_PARAM].getValue());
            }
            if ( i + 1 >= (int) params[STEPFIRST_PARAM].getValue() && i + 1 <= (int) params[STEPCOUNT_PARAM].getValue()){
                sequence.addNote(note);
            }
            currentNode = (localPrng.uniform() < params[ROUTE_PARAM + currentNode].getValue()) ? currentNode + i + 2 : currentNode + i + 1;
        }

        sequence.clampValues();
        sequence.sort();
        sequence.calculateLength();
        sequence.toClipboard();
    }

    void resetPathTraveled(){
        pathTraveled[0] = 0;
        for (size_t i = 1; i < 8; i++) pathTraveled[i] = -1;
    }
    
    void refreshSeed(){
        if (inputs[SEED_INPUT].isConnected() and (inputs[SEED_INPUT].getVoltage() != 0.f) ) {
            randomSeed = inputs[SEED_INPUT].getVoltage();
        } else {
            randomSeed = random::uniform();
        }
    }

    // Reset to the first step
    void reset(){
        step = 0;
        node = 0;
        lastNode = 0;
        lightsReset = true;
        resetPathTraveled();
        for (size_t i = 0; i < 36; i++)
            outputs[GATE_OUTPUT + i].setVoltage(0.f);
        lcdStatus.dirty = true;
        resetDelay = 0.f; // This starts the delay
    }

    bool wait1msOnReset(float sampleTime) {
        resetDelay += sampleTime;
        return((resetDelay >= 0.001f) ? true : false);
    }
    
    // Sets running to the current run status
    void setRunStatus(){
        if (runCvTrigger.process(inputs[RUN_INPUT].getVoltageSum())){
            running = !running;
            params[RUN_PARAM].setValue(running);
        }
        running = params[RUN_PARAM].getValue();
    }
        
    void setStepStatus(){
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
            resetPathTraveled();
            lightsReset = true;
            slideCounter = 0.f;
            lastOutput = outputs[CV_OUTPUT].getVoltage();
            for(int i = 0; i < stepFirst - 1; i++) {
                step++;
                nodeForward();
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

    void updateRoutes(){
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
    void setSlide(){
        slideDuration = params[SLIDE_PARAM].getValue();
        if (slideDuration > 0.00001f ) {
            slideDuration = rescale(slideDuration, 0.f, 10.f, -3.0f, 1.0f);
            slideDuration = powf(10.0f, slideDuration);
        } else {
            slideDuration = 0.f;
        }
    }
    
    void nodeForward(){
        steppedForward = false;
        
        // Refresh seed at start of sequences, and on external seeds
        // Refresh at the last minute: when about to move the second step (step == 1), not when entering the first (step == 0).
        if (step == 1){
            refreshSeed();
            prng.init(randomSeed, randomSeed);
        } else {
            if (params[SEED_MODE_PARAM].getValue() == 1.0f && inputs[SEED_INPUT].isConnected()){
                refreshSeed();
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
                        /* NOTE: This check prevents issue #21 but I don't understand why.
                           Not gonna bother to figure out this old bad code. */
                        node = 1; 
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
        lcdStatus.dirty = true;
    }
    
    void nodeBack(){
        lightsReset = true;
        node = pathTraveled[step];
        // NOTE: This conditional avoids a bizarre problem where randomSeed goes NaN.
        // Not sure what's exactly going on, not very eager to figure out this old code.
        if (step < 7) pathTraveled[step + 1] = -1; 
        lastNode = node;
        lcdStatus.dirty = true;
    }

    void updateScale(){
        if (inputs[EXT_SCALE_INPUT].isConnected()) {
            for(int i = 0; i < 12; i++) {
                scale [i] = (inputs[EXT_SCALE_INPUT].getVoltage(i) > 0.1f) ? true : false;
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
    
    void updateLights(){
        // The Seed input light
        lights[SEED_LIGHT].setBrightness( ( inputs[SEED_INPUT].getVoltage() == 0.f ) ? 0.f : 1.f );


        // Clean up by request only
        if (lightsReset) {
            for (size_t i = 0; i < 36; i++) lights[CV_LIGHT + i].setBrightness( 0.f );
            for (size_t i = 0; i < 8; i++) {
                if (pathTraveled[i] >= 0) {
                    if (pathTraveled[i] < STEP_STARTS[(int) params[STEPFIRST_PARAM].getValue()]) {
                        lights[CV_LIGHT + pathTraveled[i]].setBrightness( 0.3f );
                    } else {
                        lights[CV_LIGHT + pathTraveled[i]].setBrightness( 1.f );
                    }
                }
            }
            lightsReset = false;
        }		

        // Using an intermediary to prevent flicker
        lights[CV_LIGHT + pathTraveled[step]].setBrightness( 1.f );

        float brightness[36];
        // Light the outputs depending on amount of steps enabled
        brightness[0] = (stepFirst <= 1 && stepLast >= 1 ) ? 1.f : 0.f ;
        for (size_t i = STEP2START; i < STEP3START; i++)
            brightness[i] = (stepFirst <= 2 && stepLast >= 2 ) ? 1.f : 0.f;
        for (size_t i = STEP3START; i < STEP4START; i++)
            brightness[i] = (stepFirst <= 3 && stepLast >= 3 ) ? 1.f : 0.f;
        for (size_t i = STEP4START; i < STEP5START; i++)
            brightness[i] = (stepFirst <= 4 && stepLast >= 4 ) ? 1.f : 0.f;
        for (size_t i = STEP5START; i < STEP6START; i++)
            brightness[i] = (stepFirst <= 5 && stepLast >= 5 ) ? 1.f : 0.f;
        for (size_t i = STEP6START; i < STEP7START; i++)
            brightness[i] = (stepFirst <= 6 && stepLast >= 6 ) ? 1.f : 0.f;
        for (size_t i = STEP7START; i < STEP8START; i++)
            brightness[i] = (stepFirst <= 7 && stepLast >= 7 ) ? 1.f : 0.f;
        for (size_t i = STEP8START; i < STEP9START; i++)
            brightness[i] = (stepFirst <= 8 && stepLast >= 8 ) ? 1.f : 0.f;
        // And turn off nodes that are impossible to reach
        for (size_t i = 0; i < 36; i++)
            if (probabilities[i] == 0.f) brightness[i] = 0.f;

        for (size_t i = 0; i < 36; i++)
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
        lcdStatus.dirty = true;

        // Reset after 3 seconds since the last interactive input was touched
        if (lcdLastInteraction < (3.f / DISPLAYDIVIDER) ) {
            lcdLastInteraction += args.sampleTime;
            // Updating only once after reset
            if(lcdLastInteraction >= (3.f / DISPLAYDIVIDER) ) {
                lcdMode = DEFAULT_MODE;
                lcdStatus.dirty = true;
            }
        }

        // Default mode = pick the relevant one instead
        if(lcdMode == DEFAULT_MODE) {
            lcdMode = (params[QUANTIZE_TOGGLE_PARAM].getValue() == 0.f) ? CV_MODE : QUANTIZED_MODE;
        }

        if (lcdMode == SLIDE_MODE) {
            lcdStatus.layout = Lcd::TEXT1_AND_TEXT2_LAYOUT;
            lcdStatus.text1 = "Slide:";
            float displayDuration = slideDuration;
            if (displayDuration == 0.f)
                lcdStatus.text2 = "DISABLED";
            if (displayDuration > 0.f && displayDuration < 1.f) {
                int displayDurationMs = displayDuration * 1000;
                displayDurationMs = truncf(displayDurationMs);
                lcdStatus.text2 = std::to_string(displayDurationMs);
                lcdStatus.text2.append("ms");
            } 
            if (displayDuration >= 1.f) {
                lcdStatus.text2 = std::to_string(displayDuration);
                lcdStatus.text2.resize(4);
                lcdStatus.text2.append("s");
            }
        }

        if (lcdMode == SCALE_MODE) {
            lcdStatus.layout = Lcd::PIANO_AND_TEXT2_LAYOUT;
            if(params[SCALE_PARAM].getValue() == 0.f) {
                text = "CHROMATIC";
            } else {
                text = Quantizer::keyLcdName((int)params[KEY_PARAM].getValue());
                text.append(" ");
                text.append(Quantizer::scaleLcdName((int)params[SCALE_PARAM].getValue()));
            }
            if(inputs[EXT_SCALE_INPUT].isConnected()){
                text = "EXTERNAL";
            }
            lcdStatus.text2 = text;
            lcdStatus.pianoDisplay = scale;
        }

        if (lcdMode == QUANTIZED_MODE){
            lcdStatus.layout = Lcd::PIANO_AND_TEXT2_LAYOUT;
            lcdStatus.text2 = Quantizer::noteOctaveLcdName(outputs[CV_OUTPUT].getVoltage());
            lcdStatus.pianoDisplay = Quantizer::pianoDisplay(outputs[CV_OUTPUT].getVoltage());
        }

        if (lcdMode == CV_MODE){
            lcdStatus.layout = Lcd::TEXT1_AND_TEXT2_LAYOUT;
            text = std::to_string( outputs[CV_OUTPUT].getVoltage() );
            text.resize(5);
            lcdStatus.text1 = "";
            lcdStatus.text2 = text + "V";
        }

        if (lcdMode == MINMAX_MODE) {
            lcdStatus.layout = Lcd::TEXT1_AND_TEXT2_LAYOUT;
            if (params[QUANTIZE_TOGGLE_PARAM].getValue() == 0.f) {
                text = (params[RANGE_PARAM].getValue() == 0.f) ? std::to_string(params[MIN_PARAM].getValue()) : std::to_string(params[MIN_PARAM].getValue() - 5.f);
                text.resize(5);
                text.append("V");
            } else {
                text = (params[RANGE_PARAM].getValue() == 0.f) ? Quantizer::noteOctaveLcdName(params[MIN_PARAM].getValue() - 4.f) : Quantizer::noteOctaveLcdName(params[MIN_PARAM].getValue() - 5.f);
            }
            lcdStatus.text1 = "Min: " + text;

            if (params[QUANTIZE_TOGGLE_PARAM].getValue() == 0.f) {
                text = (params[RANGE_PARAM].getValue() == 0.f) ? std::to_string(params[MAX_PARAM].getValue()) : std::to_string(params[MAX_PARAM].getValue() - 5.f);
                text.resize(5);
                text.append("V");
            } else {
                text = (params[RANGE_PARAM].getValue() == 0.f) ? Quantizer::noteOctaveLcdName(params[MAX_PARAM].getValue() - 4.f) : Quantizer::noteOctaveLcdName(params[MAX_PARAM].getValue() - 5.f);
            }
            lcdStatus.text2 = "Max: " + text;
        }


        if (lcdMode == KNOB_MODE) {
            if (params[QUANTIZE_TOGGLE_PARAM].getValue() == 0.f) {
                lcdStatus.layout = Lcd::TEXT1_AND_TEXT2_LAYOUT;
                if (params[RANGE_PARAM].getValue() == 0.f) {
                    f = rescale( params[CV_PARAM + lastCvChanged].getValue(), 0.f, 10.f, params[MIN_PARAM].getValue(), params[MAX_PARAM].getValue());
                } else {
                    f = rescale( params[CV_PARAM + lastCvChanged].getValue(), 0.f, 10.f, params[MIN_PARAM].getValue() - 5.f, params[MAX_PARAM].getValue() - 5.f);
                }
                text = std::to_string(f);
                text.resize(5);
                lcdStatus.text1 = "";
                lcdStatus.text2 = ">" + text + "V";
            } else {
                lcdStatus.layout = Lcd::PIANO_AND_TEXT2_LAYOUT;
                validNotes = Quantizer::validNotesInScaleKey( (int)params[SCALE_PARAM].getValue() , (int)params[KEY_PARAM].getValue() );
                 if (params[RANGE_PARAM].getValue() == 0.f) {
                     f = rescale(params[CV_PARAM + lastCvChanged].getValue(), 0.f, 10.f, params[MIN_PARAM].getValue() - 4.f, params[MAX_PARAM].getValue() - 4.f);
                 } else {
                     f = rescale(params[CV_PARAM + lastCvChanged].getValue(), 0.f, 10.f, params[MIN_PARAM].getValue() - 5.f, params[MAX_PARAM].getValue() - 5.f);
                 }
                 f = Quantizer::quantize( f, validNotes);
                 lcdStatus.pianoDisplay = Quantizer::pianoDisplay(f);
                 lcdStatus.text2 = ">" + Quantizer::noteOctaveLcdName(f);
            }
        }

        if (lcdMode == ROUTE_MODE) {
            lcdStatus.layout = Lcd::TEXT1_AND_TEXT2_LAYOUT;
            relative = std::to_string( (1.f - params[ROUTE_PARAM + lastRouteChanged].getValue()) * 100.f);
            relative.resize(4);
            if (1.f - params[ROUTE_PARAM + lastRouteChanged].getValue() >= 0.9999f){
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
            if (probabilities[getUpChild(lastRouteChanged)] >= 0.9999f){
                absolute.resize(3);
                absolute.append(" %");
            } else {
                absolute.resize(4);
                absolute.append("%");
            }
            lcdStatus.text1 = relative + "/" + absolute;

            relative = std::to_string(params[ROUTE_PARAM + lastRouteChanged].getValue() * 100.f);
            relative.resize(4);
            if (params[ROUTE_PARAM + lastRouteChanged].getValue() >= 0.9999f){
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
            if (probabilities[getDownChild(lastRouteChanged)] >= 0.9999f){
                absolute.resize(3);
                absolute.append(" %");
            } else {
                absolute.resize(4);
                absolute.append("%");
            }
            lcdStatus.text2 = relative + "/" + absolute;
        }
    }

    void onReset() override {
        step = 0;
        node = 0;
        lastNode = 0;
        pathTraveled[0] = 0;
        for (size_t i = 1; i < 8; i++) pathTraveled[i] = -1;
        lightsReset = true;
        lcdMode = INIT_MODE;
        lcdStatus.layout = Lcd::TEXT1_AND_TEXT2_LAYOUT;
        lcdStatus.text1 = "MEDITATE...";
        lcdStatus.text2 = "MEDITATION.";
        lcdLastInteraction = 0.f;
        lcdStatus.dirty = true;
        resetDelay = 0.f;
    }

    void process(const ProcessArgs& args) override {
        if (processDivider.process()) {

            if (copyPortableSequence)
                exportPortableSequence();

            if (pastePortableSequence)
                importPortableSequence();


            if (randomizeCvTrigger.process(params[RANDCV_PARAM].getValue()))
                randomizeCv();
            if (randomizeRouteTrigger.process(params[RANDROUTE_PARAM].getValue()))
                randomizeRoute();
            if (resetCvTrigger.process(inputs[RESET_INPUT].getVoltageSum()) or resetButtonTrigger.process(params[RESET_PARAM].getValue()))
                reset();
            if (resetDelay >= 0.f) {
                if (wait1msOnReset(args.sampleTime)) {
                    // Done with reset
                    resetDelay = -1.f;
                } else {
                    return;
                }
            }


            if (resetCV)
                processResetCV();
            if (resetRoutes)
                processResetRoutes();
            if (routesToTop)
                processRoutesToTop();
            if (routesToBottom)
                processRoutesToBottom();
            if (routesToEqualProbability)
                processRoutesToEqualProbability();
            if (routesToBinaryTree)
                processRoutesToBinaryTree();

            setRunStatus();
            setStepStatus();

            updateRoutes();

            setSlide();

            if (steppedForward)
                nodeForward();
            if (steppedBack)
                nodeBack();

            updateScale();

            sendGateOutput(args);
            setVoltageOutput(args);
        }
        
        if (displayDivider.process()) {
            updateLights();
            updateLcd(args);
        }
    }
};





///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////





struct KnobLcd : W::Knob {
    void onDragMove(const event::DragMove& e) override {
        ParamQuantity* const paramQuantity = getParamQuantity();
        dynamic_cast<Darius*>(paramQuantity->module)->lcdLastInteraction = 0.f;
        dynamic_cast<Darius*>(paramQuantity->module)->lcdStatus.dirty = true;
        W::Knob::onDragMove(e);
    }
};


struct KnobMinMax : KnobLcd {
    void onDragMove(const event::DragMove& e) override {
        ParamQuantity* const paramQuantity = getParamQuantity();
        dynamic_cast<Darius*>(paramQuantity->module)->lcdMode = MINMAX_MODE;
        KnobLcd::onDragMove(e);
    }
};

struct KnobScale : KnobLcd {
    KnobScale() {
        snap = true;
    }
    void onDragMove(const event::DragMove& e) override {
        ParamQuantity* const paramQuantity = getParamQuantity();
        dynamic_cast<Darius*>(paramQuantity->module)->lcdMode = SCALE_MODE;
        KnobLcd::onDragMove(e);
    }
};

struct KnobSlide : KnobLcd {
    void onDragMove(const event::DragMove& e) override {
        ParamQuantity* const paramQuantity = getParamQuantity();
        dynamic_cast<Darius*>(paramQuantity->module)->lcdMode = SLIDE_MODE;
        KnobLcd::onDragMove(e);
    }
};

struct RockerSwitchHorizontalModeReset : W::RockerSwitchHorizontal {
    void onDragStart(const event::DragStart& e) override {
        ParamQuantity* const paramQuantity = getParamQuantity();
        dynamic_cast<Darius*>(paramQuantity->module)->lcdMode = DEFAULT_MODE;
        dynamic_cast<Darius*>(paramQuantity->module)->lcdLastInteraction = 0.f;
        dynamic_cast<Darius*>(paramQuantity->module)->lcdStatus.dirty = true;
        W::RockerSwitchHorizontal::onDragStart(e);
    }
};

// Rocker siwtch, horizontal, flipped for backwards compatibility.
struct RockerSwitchHorizontalFlipped : W::SvgSwitchUnshadowed {
    RockerSwitchHorizontalFlipped() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/rocker-switch-800-r.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/rocker-switch-800-l.svg")));
    }
};

// Also records the last one changed
template <class TParamWidget>
TParamWidget* createMainParam(math::Vec pos, Darius* module, int paramId, int lastChanged) {
    TParamWidget* o = new TParamWidget(module, lastChanged);
    o->box.pos = pos;
    o->app::ParamWidget::module = module;
    o->app::ParamWidget::paramId = paramId;
    o->initParamQuantity();
    return o;
}

struct KnobRoute : W::Knob {
    Darius *module;
    int lastChanged;

    KnobRoute(Darius* module, int lastChanged) {
        this->module = module;
        this->lastChanged = lastChanged;
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/knob-820-arrow.svg")));
        minAngle = 0.25 * M_PI;
        maxAngle = 0.75 * M_PI;
        W::Knob();
    }

    void onDragMove(const event::DragMove& e) override {
        module->lcdMode = ROUTE_MODE;
        module->lcdLastInteraction = 0.f;
        module->lcdStatus.dirty = true;
        module->lastRouteChanged = lastChanged;
        W::Knob::onDragMove(e);
    }
};

struct KnobTransparentCV : W::KnobTransparent {
    Darius *module;
    int lastChanged;

    KnobTransparentCV(Darius* module, int lastChanged) {
        this->module = module;
        this->lastChanged = lastChanged;
        W::KnobTransparent();
    }

    void onDragMove(const event::DragMove& e) override {
        module->lcdMode = KNOB_MODE;
        module->lcdLastInteraction = 0.f;
        module->lcdStatus.dirty = true;
        module->lastCvChanged = lastChanged;
        W::KnobTransparent::onDragMove(e);
    }
};



struct KnobLightYellowTest : W::KnobLightYellow {

};


struct DariusWidget : W::ModuleWidget {
    DariusWidget(Darius* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/faceplates/Darius.svg")));
        
        // Signature
        addChild(createWidget<W::Signature>(mm2px(Vec(120.0, 114.5f))));
        
        // Screws
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // The main area - lights, knobs and trigger outputs.
        for (size_t i = 0; i < 1; i++) {
            addChild(W::createKnobLight<W::KnobLightYellow>(mm2px(Vec( 4.5, (16.0 + (6.5 * 7) + i * 13.0))), module,
                                                         Darius::CV_LIGHT + i, Darius::CV_PARAM + i, 0.f, 10.f));
            addParam(createMainParam<KnobTransparentCV>(mm2px(Vec( 4.5, (16.0 + (6.5 * 7) + i * 13.0))), module, Darius::CV_PARAM + i, i));
            addParam(createMainParam<KnobRoute>(mm2px(Vec(14.5, (16.0 + (6.5 * 7) + i * 13.0))), module, Darius::ROUTE_PARAM + i, i));
            addDynamicOutput(mm2px(Vec( 9.5, (22.5 + (6.5 * 7) + i * 13.0))), module, Darius::GATE_OUTPUT + i, Darius::GATE_LIGHT +  i);
        }
        for (size_t i = 0; i < 2; i++) {
            addChild(W::createKnobLight<W::KnobLightYellow>(mm2px(Vec(24.5, (16.0 + (6.5 * 6) + i * 13.0))), module,
                                                         Darius::CV_LIGHT + i + STEP2START, Darius::CV_PARAM + i + STEP2START, 0.f, 10.f));
            addParam(createMainParam<KnobTransparentCV>(mm2px(Vec(24.5, (16.0 + (6.5 * 6) + i * 13.0))), module, Darius::CV_PARAM + i + STEP2START, i + STEP2START));
            addParam(createMainParam<KnobRoute>(mm2px(Vec(34.5, (16.0 + (6.5 * 6) + i * 13.0))), module, Darius::ROUTE_PARAM + i + STEP2START, i + STEP2START));
            addDynamicOutput(mm2px(Vec(29.5, (22.5 + (6.5 * 6) + i * 13.0))), module, Darius::GATE_OUTPUT + i + STEP2START, Darius::GATE_LIGHT +  i + STEP2START);
        }
        for (size_t i = 0; i < 3; i++) {
            addChild(W::createKnobLight<W::KnobLightYellow>(mm2px(Vec(44.5, (16.0 + (6.5 * 5) + i * 13.0))), module,
                                                         Darius::CV_LIGHT + i + STEP3START, Darius::CV_PARAM + i + STEP3START, 0.f, 10.f));
            addParam(createMainParam<KnobTransparentCV>(mm2px(Vec(44.5, (16.0 + (6.5 * 5) + i * 13.0))), module, Darius::CV_PARAM + i + STEP3START, i + STEP3START));
            addParam(createMainParam<KnobRoute>(mm2px(Vec(54.5, (16.0 + (6.5 * 5) + i * 13.0))), module, Darius::ROUTE_PARAM + i + STEP3START, i + STEP3START));
            addDynamicOutput(mm2px(Vec(49.5, (22.5 + (6.5 * 5) + i * 13.0))), module, Darius::GATE_OUTPUT + i + STEP3START, Darius::GATE_LIGHT +  i + STEP3START);
        }
        for (size_t i = 0; i < 4; i++) {
            addChild(W::createKnobLight<W::KnobLightYellow>(mm2px(Vec(64.5, (16.0 + (6.5 * 4) + i * 13.0))), module,
                                                         Darius::CV_LIGHT + i + STEP4START, Darius::CV_PARAM + i + STEP4START, 0.f, 10.f));
            addParam(createMainParam<KnobTransparentCV>(mm2px(Vec(64.5, (16.0 + (6.5 * 4) + i * 13.0))), module, Darius::CV_PARAM + i + STEP4START, i + STEP4START));
            addParam(createMainParam<KnobRoute>(mm2px(Vec(74.5, (16.0 + (6.5 * 4) + i * 13.0))), module, Darius::ROUTE_PARAM + i + STEP4START, i + STEP4START));
            addDynamicOutput(mm2px(Vec(69.5, (22.5 + (6.5 * 4) + i * 13.0))), module, Darius::GATE_OUTPUT + i + STEP4START, Darius::GATE_LIGHT +  i + STEP4START);
        }
        for (size_t i = 0; i < 5; i++) {
            addChild(W::createKnobLight<W::KnobLightYellow>(mm2px(Vec(84.5, (16.0 + (6.5 * 3) + i * 13.0))), module,
                                                         Darius::CV_LIGHT + i + STEP5START, Darius::CV_PARAM + i + STEP5START, 0.f, 10.f));
            addParam(createMainParam<KnobTransparentCV>(mm2px(Vec(84.5, (16.0 + (6.5 * 3) + i * 13.0))), module, Darius::CV_PARAM + i + STEP5START, i + STEP5START));
            addParam(createMainParam<KnobRoute>(mm2px(Vec(94.5, (16.0 + (6.5 * 3) + i * 13.0))), module, Darius::ROUTE_PARAM + i + STEP5START, i + STEP5START));
            addDynamicOutput(mm2px(Vec(89.5, (22.5 + (6.5 * 3) + i * 13.0))), module, Darius::GATE_OUTPUT + i + STEP5START, Darius::GATE_LIGHT +  i + STEP5START);
        }
        for (size_t i = 0; i < 6; i++) {
            addChild(W::createKnobLight<W::KnobLightYellow>(mm2px(Vec(104.5, (16.0 + (6.5 * 2) + i * 13.0))), module,
                                                         Darius::CV_LIGHT + i + STEP6START, Darius::CV_PARAM + i + STEP6START, 0.f, 10.f));
            addParam(createMainParam<KnobTransparentCV>(mm2px(Vec(104.5, (16.0 + (6.5 * 2) + i * 13.0))), module, Darius::CV_PARAM + i + STEP6START, i + STEP6START));
            addParam(createMainParam<KnobRoute>(mm2px(Vec(114.5, (16.0 + (6.5 * 2) + i * 13.0))), module, Darius::ROUTE_PARAM + i + STEP6START, i + STEP6START));
            addDynamicOutput(mm2px(Vec(109.5, (22.5 + (6.5 * 2) + i * 13.0))), module, Darius::GATE_OUTPUT + i + STEP6START, Darius::GATE_LIGHT +  i + STEP6START);
        }
        for (size_t i = 0; i < 7; i++) {
            addChild(W::createKnobLight<W::KnobLightYellow>(mm2px(Vec(124.5, (16.0 + (6.5 * 1) + i * 13.0))), module,
                                                         Darius::CV_LIGHT + i + STEP7START, Darius::CV_PARAM + i + STEP7START, 0.f, 10.f));
            addParam(createMainParam<KnobTransparentCV>(mm2px(Vec(124.5, (16.0 + (6.5 * 1) + i * 13.0))), module, Darius::CV_PARAM + i + STEP7START, i + STEP7START));
            addParam(createMainParam<KnobRoute>(mm2px(Vec(134.5, (16.0 + (6.5 * 1) + i * 13.0))), module, Darius::ROUTE_PARAM + i + STEP7START, i + STEP7START));
            addDynamicOutput(mm2px(Vec(129.5, (22.5 + (6.5 * 1) + i * 13.0))), module, Darius::GATE_OUTPUT + i + STEP7START, Darius::GATE_LIGHT +  i + STEP7START);
        }
        for (size_t i = 0; i < 8; i++) {
            addChild(W::createKnobLight<W::KnobLightYellow>(mm2px(Vec(144.5, (16.0 + (6.5 * 0) + i * 13.0))), module,
                                                         Darius::CV_LIGHT + i + STEP8START, Darius::CV_PARAM + i + STEP8START, 0.f, 10.f));
            addParam(createMainParam<KnobTransparentCV>(mm2px(Vec(144.5, (16.0 + (6.5 * 0) + i * 13.0))), module, Darius::CV_PARAM + i + STEP8START, i + STEP8START));
            addDynamicOutput(mm2px(Vec(149.5, (22.5 + (6.5 * 0) + i * 13.0))), module, Darius::GATE_OUTPUT + i + STEP8START, Darius::GATE_LIGHT +  i + STEP8START);
        }
        
        // Step < ^ v >
        addStaticInput(mm2px(Vec(4.5, 22.5)), module, Darius::STEP_BACK_INPUT);
        addStaticInput(mm2px(Vec(14.5, 18.0)), module, Darius::STEP_UP_INPUT);
        addStaticInput(mm2px(Vec(14.5, 27.0)), module, Darius::STEP_DOWN_INPUT);
        addStaticInput(mm2px(Vec(24.5, 22.5)), module, Darius::STEP_INPUT);
        addParam(createParam<W::ButtonMomentary>(mm2px(Vec(24.5, 32.5)), module, Darius::STEP_PARAM));
        
        // Run
        addStaticInput(mm2px(Vec(4.5, 42.5)), module, Darius::RUN_INPUT);
        addParam(createParam<W::Button>(mm2px(Vec(14.5, 42.5)), module, Darius::RUN_PARAM));
        
        // Reset
        addStaticInput(mm2px(Vec(24.5, 42.5)), module, Darius::RESET_INPUT);
        addParam(createParam<W::ButtonMomentary>(mm2px(Vec(34.5, 42.5)), module, Darius::RESET_PARAM));
        
        // Step count & First step
        addParam(createParam<W::KnobSnap>(mm2px(Vec(44.5, 22.5)), module, Darius::STEPFIRST_PARAM));
        addParam(createParam<W::KnobSnap>(mm2px(Vec(54.5, 22.5)), module, Darius::STEPCOUNT_PARAM));
        
        // Randomize
        addParam(createParam<W::ButtonMomentary>(mm2px(Vec(64.5, 22.5)), module, Darius::RANDCV_PARAM));
        addParam(createParam<W::ButtonMomentary>(mm2px(Vec(74.5, 22.5)), module, Darius::RANDROUTE_PARAM));
        
        // Seed
        addParam(createParam<W::RockerSwitchVertical>(mm2px(Vec(103.0, 112.0)), module, Darius::SEED_MODE_PARAM));
        addStaticInput(mm2px(Vec(109.5, 112.0)), module, Darius::SEED_INPUT);
        addChild(createLight<W::StatusLightInput>(mm2px(Vec(107.7, 120.4)), module, Darius::SEED_LIGHT));

        // Output area //////////////////

        // LCD
        Lcd::LcdWidget<Darius> *lcd = new Lcd::LcdWidget<Darius>(module, "MEDITATE...", "MEDITATION.");
        lcd->box.pos = mm2px(Vec(10.3f, 106.7f));
        addChild(lcd);

        // Quantizer toggle
        addParam(createParam<RockerSwitchHorizontalModeReset>(mm2px(Vec(11.1, 99.7)), module, Darius::QUANTIZE_TOGGLE_PARAM));

        // Voltage Range
        addParam(createParam<RockerSwitchHorizontalFlipped>(mm2px(Vec(28.0, 118.8)), module, Darius::RANGE_PARAM));

        // Min & Max
        addParam(createParam<KnobMinMax>(mm2px(Vec(49.5f, 112.f)), module, Darius::MIN_PARAM)); 
        addParam(createParam<KnobMinMax>(mm2px(Vec(59.5f, 112.f)), module, Darius::MAX_PARAM)); 

        // Quantizer Key & Scale
        addParam(createParam<KnobScale>(mm2px(Vec(49.5f, 99.f)), module, Darius::KEY_PARAM));
        addParam(createParam<KnobScale>(mm2px(Vec(59.5f, 99.f)), module, Darius::SCALE_PARAM));

        // External Scale
        addStaticInput(mm2px(Vec(69.5, 99.0)), module, Darius::EXT_SCALE_INPUT);

        // Slide
        addParam(createParam<KnobSlide>(mm2px(Vec(69.5, 112.0)), module, Darius::SLIDE_PARAM));

        // Output!
        addStaticOutput(mm2px(Vec(79.5, 112.0)), module, Darius::GLOBAL_GATE_OUTPUT);
        addStaticOutput(mm2px(Vec(89.5, 112.0)), module, Darius::CV_OUTPUT);
    }


    struct CopyPortableSequenceItem : MenuItem {
        Darius *module;
        void onAction(const event::Action &e) override {
            module->copyPortableSequence = true;
        }
    };

    struct PastePortableSequenceItem : MenuItem {
        Darius *module;
        void onAction(const event::Action &e) override {
            module->pastePortableSequence = true;
        }
    };

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

        CopyPortableSequenceItem *copyPortableSequenceItem = createMenuItem<CopyPortableSequenceItem>("Copy one possible route as Portable Sequence");
        copyPortableSequenceItem->module = module;
        menu->addChild(copyPortableSequenceItem);

        PastePortableSequenceItem *pastePortableSequenceItem = createMenuItem<PastePortableSequenceItem>("Paste Portable Sequence (identical values per step)");
        pastePortableSequenceItem->module = module;
        menu->addChild(pastePortableSequenceItem);

        MenuLabel *pasteNotes = createMenuLabel<MenuLabel>("After pasting, set MIN/MAX knobs to maximum range");
        menu->addChild(pasteNotes);
      
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
    }
};

} // namespace Darius

Model* modelDarius = createModel<Darius::Darius, Darius::DariusWidget>("Darius");
