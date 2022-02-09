/*  Copyright (C) 2019-2020 Aria Salvatrice
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

/* Quatherina's Quality Quad Quantizer, Quack, Q<
   All three modules are the same, with different widgets.
   Yeah kinda wasteful but hey, still faster than most quantizers in the library.
*/ 

#include "plugin.hpp"
#include "quantizer.hpp"
#include "lcd.hpp"
#include "javascript.hpp"
#include "javascript-libraries.hpp"
#include "portablesequence.hpp"
#include "polyexternalscale.hpp"

namespace Qqqq {

enum LcdModes {
    INIT_MODE,
    LOAD_MODE,
    READY_MODE,
    SCALE_MODE,
    SCALING_MODE,
    OFFSET_MODE,
    TRANSPOSE_MODE,
    TRANSPOSE_TYPE_MODE,
    SH_MODE,
    VISUALIZE_MODE
};

// Nope, not gonna give you a placebo "HIGH CPU" right click option unless people
// raise valid complaints about this divider. 
const int PROCESSDIVIDER = 32;
const int LCDDIVIDER = 512;

struct Qqqq : Module {
    enum ParamIds {
        ENUMS(NOTE_PARAM, 12),
        ENUMS(SCALING_PARAM, 4),
        ENUMS(OFFSET_PARAM, 4),
        ENUMS(TRANSPOSE_PARAM, 4),
        ENUMS(TRANSPOSE_MODE_PARAM, 4),
        ENUMS(SH_MODE_PARAM, 4),
        ENUMS(VISUALIZE_PARAM, 4),
        ENUMS(SCENE_BUTTON_PARAM, 16),
        KEY_PARAM,
        SCALE_PARAM,
        KEYBOARD_INPUT_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        ENUMS(CV_INPUT, 4),
        ENUMS(SH_INPUT, 4),
        EXT_SCALE_INPUT,
        SCENE_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        ENUMS(CV_OUTPUT, 4),
        EXT_SCALE_OUTPUT,
        ENUMS(DEBUG_OUTPUT, 4),
        NUM_OUTPUTS
    };
    enum LightIds {
        EXPANDER_IN_LIGHT,
        EXPANDER_OUT_LIGHT,
        NUM_LIGHTS
    };

    bool lastExtInConnected = false;
    bool sceneChanged = false;
    bool isExpander = false;
    bool lastIsExpander = false;
    bool sceneTrigSelection = false;
    bool initialized = false;
    int lcdMode = INIT_MODE;
    int scene = 0;
    int lastScene = 0;
    int lastScalingKnobTouchedId = 0;
    int lastOffsetKnobTouchedId = 0;
    int lastTransposeKnobTouchedId = 0;
    int lastTransposeModeTouchedId = 0;
    int lastShTouchedId = 0;
    size_t initCounter = 0;
    float lcdLastInteraction = 0.f;
    float lastKeyKnob = 0.f;
    float lastScaleKnob = 2.f;
    std::array<std::array<bool, 12>, 16> scale;
    std::array<bool, 12> lastExternalScale;
    std::array<bool, 12> litKeys;
    std::array<bool, 12> receivedExpanderScale;
    std::array<bool, 12> lastReceivedExpanderScale;
    std::array<std::array<float, 16>, 4> inputVoltage;
    std::array<std::array<float, 16>, 4> shVoltage;
    std::array<int, 4> inputChannels;
    std::array<int, 4> shChannels;
    Lcd::LcdStatus lcdStatus;
    dsp::ClockDivider processDivider;
    dsp::ClockDivider lcdDivider;
    dsp::SchmittTrigger shTrigger[4];
    dsp::SchmittTrigger sceneSelectionTrigger;
    PolyExternalScale::PESExpanderMessage pesExpanderProducerMessage;
    PolyExternalScale::PESExpanderMessage pesExpanderConsumerMessage;
    
    Qqqq() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        // Default to C Minor, to have a visually obvious mix of white and black keys
        configParam(NOTE_PARAM +  0, 0.f, 1.f, 1.f, "C");
        configParam(NOTE_PARAM +  1, 0.f, 1.f, 0.f, "C#");
        configParam(NOTE_PARAM +  2, 0.f, 1.f, 1.f, "D");
        configParam(NOTE_PARAM +  3, 0.f, 1.f, 1.f, "D#");
        configParam(NOTE_PARAM +  4, 0.f, 1.f, 0.f, "E");
        configParam(NOTE_PARAM +  5, 0.f, 1.f, 1.f, "F");
        configParam(NOTE_PARAM +  6, 0.f, 1.f, 0.f, "F#");
        configParam(NOTE_PARAM +  7, 0.f, 1.f, 1.f, "G");
        configParam(NOTE_PARAM +  8, 0.f, 1.f, 1.f, "G#");
        configParam(NOTE_PARAM +  9, 0.f, 1.f, 0.f, "A");
        configParam(NOTE_PARAM + 10, 0.f, 1.f, 1.f, "A#");
        configParam(NOTE_PARAM + 11, 0.f, 1.f, 0.f, "B");
        configParam(KEY_PARAM, 0.f, 11.f, 0.f, "Key");
        configParam(SCALE_PARAM, 0.f, (float) Quantizer::NUM_SCALES - 1, 2.f, "Scale");
        for (size_t i = 0; i < 4; i++){
            configParam(SCALING_PARAM + i, -100.f, 300.f, 100.f, "Scaling", "%");
            configParam(OFFSET_PARAM + i, -10.f, 10.f, 0.f, "Offset", "V");
            configParam(TRANSPOSE_PARAM + i, -12.f, 12.f, 0.f, "Transpose");
            configParam(TRANSPOSE_MODE_PARAM + i, 0.f, 2.f, 0.f, "Transpose Mode");
            configParam(SH_MODE_PARAM + i, 0.f, 1.f, 0.f, "S&H / T&H Toggle");
        }
        // This is a ugly hack to ensure Quack gets visualized, as it lacks a toggle.
        configParam(VISUALIZE_PARAM + 0, 0.f, 1.f, 1.f, "Visualize on Piano");
        configParam(VISUALIZE_PARAM + 1, 0.f, 1.f, 0.f, "Visualize on Piano");
        configParam(VISUALIZE_PARAM + 2, 0.f, 1.f, 0.f, "Visualize on Piano");
        configParam(VISUALIZE_PARAM + 3, 0.f, 1.f, 0.f, "Visualize on Piano");
        configParam(SCENE_BUTTON_PARAM, 0.f, 1.f, 0.f, "Scene #1");
        for (size_t i = 1; i < 16; i++) configParam(SCENE_BUTTON_PARAM + i, 0.f, 1.f, 0.f, "Scene #" + std::to_string(i + 1));
        processDivider.setDivision(PROCESSDIVIDER);
        lcdDivider.setDivision(LCDDIVIDER);
        lcdMode = INIT_MODE;
        lcdLastInteraction = 0.f;
        lcdStatus.text1 = " Q- ...";
        lcdStatus.layout = Lcd::TEXT1_LAYOUT;
        // Initialize
        for (size_t i = 0; i < 16; i++) { for (int j = 0; j < 12; j++) { scale[i][j] = false; }}
        // C Minor in first scene
        scale[0][0] = true; scale[0][2] = true; scale[0][3] = true; scale[0][5] = true; scale[0][7] = true; scale[0][8] = true; scale[0][10] = true;
        // Expander
        leftExpander.producerMessage = &pesExpanderProducerMessage;
        leftExpander.consumerMessage = &pesExpanderConsumerMessage;
    }


    json_t* dataToJson() override {
        json_t* rootJ = json_object();

        json_object_set_new(rootJ, "sceneTrigSelection", json_boolean(sceneTrigSelection));
        json_object_set_new(rootJ, "scene", json_integer(scene));

        json_t* scenesJ = json_array();
        for (size_t i = 0; i < 16; i++) {
            json_t* sceneJ = json_array();
            for (int j = 0; j < 12; j++) {
                json_array_append_new(sceneJ, json_boolean(scale[i][j]));
            }
            json_array_append_new(scenesJ, sceneJ);
        }
        json_object_set_new(rootJ, "scenes", scenesJ);
        
        return rootJ;
    }

    void dataFromJson(json_t* rootJ) override {
        json_t* sceneTrigSelectionJ = json_object_get(rootJ, "sceneTrigSelection");
        if (sceneTrigSelectionJ) sceneTrigSelection = json_boolean_value(sceneTrigSelectionJ);

        json_t* sceneJ = json_object_get(rootJ, "scene");
        if (sceneJ) scene = json_integer_value(sceneJ);

        json_t* scenesJ = json_object_get(rootJ, "scenes");
        if (scenesJ){
            for (size_t i = 0; i < 16; i++) {
                json_t* sceneJ = json_array_get(scenesJ, i);
                if (sceneJ) {
                    for (int j = 0; j < 12; j++) {
                        json_t* noteJ = json_array_get(sceneJ, j);
                        scale[i][j] = json_boolean_value(noteJ);
                    }
                }
            }
        }
        updateScene();
        scaleToPiano();
    }

    void onReset() override {
        for (size_t i = 1; i < 16; i++) {
            for (int j = 0; j < 12; j++) {
                scale[i][j] = false;
            }
            params[SCENE_BUTTON_PARAM + i].setValue(0.f);
        }
        params[SCENE_BUTTON_PARAM + 0].setValue(1.f);
        // C Minor in first scene
        scale[0][0] = true; scale[0][2] = true; scale[0][3] = true; scale[0][5] = true; scale[0][7] = true; scale[0][8] = true; scale[0][10] = true;
        scene = 0;
        scaleToPiano();
        lcdStatus.text1 = " Q- ???";
        lcdLastInteraction = 0.f;
        lcdMode = INIT_MODE;
        lcdStatus.dirty = true;
        initialized = false;
        initCounter = 0;
    }

    void onRandomize() override {
        for (size_t i = 0; i < 16; i++) {
            for (int j = 0; j < 12; j++) {
                // Should produce about 7 notes per scale
                scale[i][j] = (random::uniform() > 0.42f) ? true : false;
            }
            params[SCENE_BUTTON_PARAM + i].setValue(0.f);
        }
        scene = 0;
        params[SCENE_BUTTON_PARAM + 0].setValue(1.f);
        lcdStatus.text1 = " Q- !!!";
        lcdLastInteraction = 0.f;
        lcdMode = INIT_MODE;
        lcdStatus.dirty = true;
    }

    // Chords, not portable sequences. Format is like:
    // [[0,4,7],[2,6,9],[4,8,11],[5,9,12],[7,11,2],[9,1,4],[11,3,6]]
    void importJson(const char* &jsonC) {
        json_error_t error;
        json_t* rootJ = json_loads(jsonC, 0, &error);
        if (!rootJ) {
            lcdStatus.text1 = "!! ERROR !!";
            lcdLastInteraction = 0.f;
            lcdMode = INIT_MODE;
            lcdStatus.dirty = true;
        } else {
            for (size_t i = 0; i < 16; i++) {
                for (int j = 0; j < 12; j++) {
                    scale[i][j] = false;
                }
            }
            size_t scenesJSize = json_array_size(rootJ);
            if (scenesJSize > 16) scenesJSize = 16;
            for (size_t i = 0; i < scenesJSize; i++) {
                json_t* scaleJ = json_array_get(rootJ, i);
                size_t scaleJSize = json_array_size(scaleJ);
                for (size_t j = 0; j < scaleJSize; j++) {
                    json_t* noteJ = json_array_get(scaleJ, j);
                    int note = json_integer_value(noteJ);
                    scale[i][note] = true;
                }
            }
            scaleToPiano();
            lcdStatus.text1 = " Imported!";
            lcdLastInteraction = 0.f;
            lcdMode = INIT_MODE;
            lcdStatus.dirty = true;
            for (size_t i = 1; i < 16; i++) {
                params[SCENE_BUTTON_PARAM + i].setValue(0.f);
            }
            scene = 0;
            params[SCENE_BUTTON_PARAM + 0].setValue(1.f);
            scaleToPiano();
        }
    }

    // Returns the last non-empty scene
    int getLastScene() {
       for (int i = 15; i >=0; i--) {
            for (int j = 0; j < 12; j++) {
                if (scale[i][j] == true) return i;
            }
        }
        return 0;
    }

    // Widget calls this directly
    void importLeadSheet(std::string text){
        Javascript::Runtime js;
        js.evaluateString(JavascriptLibraries::TONALJS);
        js.evaluateString(JavascriptLibraries::TOKENIZE);
        js.evaluateString(JavascriptLibraries::TOSCALEPOSITION);
        js.evaluateString(JavascriptLibraries::PARSEASLEADSHEET);
        js.evaluateString(JavascriptLibraries::LEADSHEETTOQQQQ);
        js.evaluateString("results = leadsheetToQqqq('" + text + "')");
        const char* results = js.readVariableAsChar("results");
        importJson(results);
    }

    // Widget calls this directly
    void importRomanNumeral(std::string text){
        Javascript::Runtime js;
        std::string tonic = Quantizer::keyLcdName((int) params[KEY_PARAM].getValue());
        js.evaluateString(JavascriptLibraries::TONALJS);
        js.evaluateString(JavascriptLibraries::TOKENIZE);
        js.evaluateString(JavascriptLibraries::TOSCALEPOSITION);
        js.evaluateString(JavascriptLibraries::PARSEASLEADSHEET);
        js.evaluateString(JavascriptLibraries::ROMANTOQQQQ);
        js.evaluateString("results = romanToQqqq('" + tonic + "', '" + text + "')");
        const char* results = js.readVariableAsChar("results");
        importJson(results);
    }

    // Widget calls this directly
    void copyPortableSequence(){
        PortableSequence::Sequence sequence;
        sequence.length = (float) getLastScene() + 1;
       for (int i = 0; i <= getLastScene(); i++) {
            for (int j = 0; j < 12; j++) {
                if (scale[i][j] == true) {
                    PortableSequence::Note note;
                    note.length = 1.f;
                    note.start = (float) i;
                    note.pitch = (float) j * 1.f / 12.f;
                    sequence.addNote(note);
                }
            }
        }
        sequence.toClipboard();
        lcdStatus.text1 = "  Copied!";
        lcdLastInteraction = 0.f;
        lcdMode = INIT_MODE;
        lcdStatus.dirty = true;
    }

    // Widget calls this directly
    void pastePortableSequence(){
        PortableSequence::Sequence sequence;
        sequence.fromClipboard();
        sequence.sort();

        if (sequence.notes.size() < 1) return;

        // Reset scales
        for (size_t i = 0; i < 16; i++) {
            for (int j = 0; j < 12; j++) {
                scale[i][j] = false;
            }
        }

        int position = 0;
        for (size_t i = 0; i < 16; i++) {
            float start = sequence.notes[position].start;
            int remaining = sequence.notes.size() - position;
            if (remaining > 0) {
                for (size_t j = 0; j < sequence.notes.size(); j++ ) {
                    if (sequence.notes[j].start == start) {
                        int note = (int) (sequence.notes[j].pitch * 12.f + 60.f) % 12;
                        scale[i][note] = true;
                        position++;
                    }
                }
            }
        }
        scene = 0;
        params[SCENE_BUTTON_PARAM + 0].setValue(1.f);
        scaleToPiano();
        lcdStatus.text1 = "  Pasted!";
        lcdLastInteraction = 0.f;
        lcdMode = INIT_MODE;
        lcdStatus.dirty = true;
    }

    void copyScenePortableSequence(int slot){
        DEBUG("COPY %d", slot);
        PortableSequence::Sequence sequence;
        sequence.length = 1.f;
        for (int j = 0; j < 12; j++) {
            if (scale[slot][j] == true) {
                PortableSequence::Note note;
                note.length = 1.f;
                note.start = 0.f;
                note.pitch = (float) j * 1.f / 12.f;
                sequence.addNote(note);
            }
        }
        sequence.toClipboard();
        lcdStatus.text1 = "  Copied!";
        lcdLastInteraction = 0.f;
        lcdMode = INIT_MODE;
        lcdStatus.dirty = true;
    }

    void pasteScenePortableSequence(int slot){
        DEBUG("PASTE %d", slot);
        PortableSequence::Sequence sequence;
        sequence.fromClipboard();
        if (sequence.notes.size() <= 0) return;
        for (size_t i = 0; i < 12; i++) scale[slot][i] = false;
        for (size_t i = 0; i < sequence.notes.size(); i++){
            int note = (int) (sequence.notes[i].pitch * 12.f + 60.f) % 12;
            scale[slot][note] = true;
        }
        scaleToPiano();
        lcdStatus.text1 = "  Pasted!";
        lcdLastInteraction = 0.f;
        lcdMode = INIT_MODE;
        lcdStatus.dirty = true;
    }

    void updateExpander(){
        if ((leftExpander.module and leftExpander.module->model == modelQqqq)
        ||  (leftExpander.module and leftExpander.module->model == modelQuack)
        ||  (leftExpander.module and leftExpander.module->model == modelQ)
        ||  (leftExpander.module and leftExpander.module->model == modelQuale)) {
            // We are an expander
            lights[EXPANDER_IN_LIGHT].setBrightness(1.f);
            PolyExternalScale::PESExpanderMessage *message = (PolyExternalScale::PESExpanderMessage*) leftExpander.consumerMessage;
            for (size_t i = 0; i < 12; i++) receivedExpanderScale[i] = message->scale[i];
            isExpander = true;
        } else {
            // We are not an expander
            lights[EXPANDER_IN_LIGHT].setBrightness(0.f);
            isExpander = false;
        }

        if ((rightExpander.module and rightExpander.module->model == modelQqqq)
        ||  (rightExpander.module and rightExpander.module->model == modelQuack)
        ||  (rightExpander.module and rightExpander.module->model == modelQ)
        ||  (rightExpander.module and rightExpander.module->model == modelQuale)) {
            // We have an expander
            lights[EXPANDER_OUT_LIGHT].setBrightness(1.f);
            PolyExternalScale::PESExpanderMessage *message = (PolyExternalScale::PESExpanderMessage*) rightExpander.module->leftExpander.producerMessage;			
            for (size_t i = 0; i < 12; i++) message->scale[i] = scale[scene][i];
            rightExpander.module->leftExpander.messageFlipRequested = true;
        } else {
            // We have no expander
            lights[EXPANDER_OUT_LIGHT].setBrightness(0.f);
        }

    }


    // Sets the scene. The CV input overrides the buttons.
    void updateScene() {

        // Voltage selection
        if (inputs[SCENE_INPUT].isConnected() && ! sceneTrigSelection && inputs[SCENE_INPUT].getVoltageSum() >= 0.f) {
            scene = (int) rescale(inputs[SCENE_INPUT].getVoltageSum(), 0.f, 10.f, 0.f, 15.2f);
            if (scene != lastScene) sceneChanged = true;
        }

        // Trig selection
        if (inputs[SCENE_INPUT].isConnected() && sceneTrigSelection && sceneSelectionTrigger.process(inputs[SCENE_INPUT].getVoltageSum()) ) {
            scene++;
            if (scene > getLastScene()) scene = 0;
            if (scene != lastScene) sceneChanged = true;
        }

        // Button selection
        if (! inputs[SCENE_INPUT].isConnected()) {
           for (int i = 0; i < 16; i++) {
                if ( params[SCENE_BUTTON_PARAM + i].getValue() == 1.f && i != lastScene ) {
                    scene = i;
                    sceneChanged = true;
                    for (int j = 0; j < 16; j++) {
                        // Turn off the other buttons
                        if (j != scene) params[SCENE_BUTTON_PARAM + j].setValue(0.f);
                    }
                }
            }
        }

        // You shouldn't be able to turn on multiple scenes at once, or turn off the current one
       for (int i = 0; i < 16; i++) params[SCENE_BUTTON_PARAM + i].setValue( (i == scene) ? 1.f : 0.f );

        lastScene = scene;
    }


    // Update the piano display to match the state of the internal scale if necessary
    void scaleToPiano() {
        for (size_t i = 0; i < 12; i++) {
            params[NOTE_PARAM + i].setValue((scale[scene][i]) ? 1.f : 0.f);
        }
    }


    // Update the internal scale to match the state of the piano display
    void pianoToScale() {
        for (size_t i = 0; i < 12; i++) scale[scene][i] = (params[NOTE_PARAM + i].getValue() == 1.f)  ? true : false;
    }


    // The last control touched always has the last word.
    void updateScale() {

        // Initialize
        if (!initialized) {
            lastKeyKnob = params[KEY_PARAM].getValue();
            lastScaleKnob = params[SCALE_PARAM].getValue();
            for (size_t i = 0; i < 12; i++) lastExternalScale[i] = (inputs[EXT_SCALE_INPUT].getVoltage(i) > 0.1f) ? true : false;

            initCounter++;
            // We need a few iterations or else other modules might not be initialized
            if (initCounter > 32) initialized = true;
        }

        // Scene: has it changed?
        if (sceneChanged) {
            scaleToPiano();
            sceneChanged = false;
        }

        // Expander: has it just been connected, or sent something new?
        if (isExpander && initialized) {
            if ((receivedExpanderScale != lastReceivedExpanderScale) || !lastIsExpander) {
                scale[scene] = receivedExpanderScale;
                scaleToPiano();
            }
        }
        lastIsExpander = isExpander;
        lastReceivedExpanderScale = receivedExpanderScale;

        // External scale: was it just connected?
        if (!lastExtInConnected && inputs[EXT_SCALE_INPUT].isConnected() && initialized) {
            for (size_t i = 0; i < 12; i++){
                scale[scene][i] = (inputs[EXT_SCALE_INPUT].getVoltage(i) > 0.1f) ? true : false;
            }
            scaleToPiano();
        }

        // External scale: has it changed?
        std::array<bool, 12> currentExternalScale;
        if (inputs[EXT_SCALE_INPUT].isConnected() && initialized) {
            for (size_t i = 0; i < 12; i++) currentExternalScale[i] = (inputs[EXT_SCALE_INPUT].getVoltage(i) > 0.1f) ? true : false;
            if (currentExternalScale != lastExternalScale) {
                lastExternalScale = currentExternalScale;
                scale[scene] = currentExternalScale;
                scaleToPiano();
            }
        }
        lastExtInConnected = inputs[EXT_SCALE_INPUT].isConnected();

        // Knobs: have they moved?
        if ( (lastKeyKnob != params[KEY_PARAM].getValue()) || (lastScaleKnob != params[SCALE_PARAM].getValue()) ) {
            scale[scene] = Quantizer::validNotesInScaleKey(params[SCALE_PARAM].getValue(), params[KEY_PARAM].getValue());
            scaleToPiano();
        }
        lastKeyKnob = params[KEY_PARAM].getValue();
        lastScaleKnob = params[SCALE_PARAM].getValue();

        // Piano display: has it changed?
        pianoToScale();
    }


    void updateExternalOutput() {
        if (outputs[EXT_SCALE_OUTPUT].isConnected()){
            for (int i = 0; i < 12; i++) outputs[EXT_SCALE_OUTPUT].setVoltage( (scale[scene][i]) ? 8.f : 0.f, i);
            outputs[EXT_SCALE_OUTPUT].setChannels(12);
        }
    }


    void cleanLitKeys() {
        for (size_t i =  0; i < 12; i++) litKeys[i] = false; 
    }


    // When there is no CV input, use the column to the left instead.
    void processInputs() {
        inputChannels[0] = inputs[CV_INPUT + 0].getChannels();
       for (int i = 0; i < inputChannels[0]; i++) inputVoltage[0][i] = inputs[CV_INPUT + 0].getVoltage(i);

        for (size_t i = 1; i < 4; i++) {
            inputChannels[i] = inputs[CV_INPUT + i].getChannels();
            if (inputChannels[i] > 0) {
                for (int j = 0; j < inputChannels[i]; j++) inputVoltage[i][j] = inputs[CV_INPUT + i].getVoltage(j);
            } else {
                inputChannels[i] = inputChannels[i - 1];
                inputVoltage[i] = inputVoltage[i - 1];
            }
        }
    }


    void processQuantizerColumn(int col){
        std::array<float, 16> voltage = inputVoltage[col];
        int channels = inputChannels[col];
        bool sh = false;
        
        // Stop if no output while visualization is not enabled
        if (! outputs[CV_OUTPUT + col].isConnected() && params[VISUALIZE_PARAM + col].getValue() == 0.f ) return;

        // S&H
        if (inputs[SH_INPUT + col].isConnected()){
            if (params[SH_MODE_PARAM].getValue() == 0.f) {
                // S&H mode
                if (shTrigger[col].process(inputs[SH_INPUT + col].getVoltageSum())) sh = true;
            } else {
                // T&H mode
                if (inputs[SH_INPUT + col].getVoltageSum() > 1.0f) sh = true;
            }
        } else {
            // No S&H input means that in effect we T&H every sample
            sh = true;
        }

        if (sh) shChannels[col] = channels;

        // Iterate channels
       for (int i = 0; i < channels; i++) {

            // Only process if S&H this sample
            if (sh) {
                // Scale and offset
                voltage[i] = voltage[i] * params[SCALING_PARAM + col].getValue() / 100.f;
                voltage[i] = voltage[i] + params[OFFSET_PARAM + col].getValue();
                if (params[TRANSPOSE_MODE_PARAM + col].getValue() == 0.f) {
                    // Quantize in transpose mode 0: Octaves
                    voltage[i] = Quantizer::quantize(clamp(voltage[i], -5.f, 5.f), scale[scene]);
                    for (size_t j = 0; j < abs((int) params[TRANSPOSE_PARAM + col].getValue()); j++ ) {
                        if (params[TRANSPOSE_PARAM + col].getValue() > 0.f && voltage[i] <= 5.f) {
                            voltage[i] += 1.f;
                        }
                        if (params[TRANSPOSE_PARAM + col].getValue() < 0.f && voltage[i] >= -5.f) {
                            voltage[i] -= 1.f;
                        }
                    }
                }
                if (params[TRANSPOSE_MODE_PARAM + col].getValue() == 1.f) {
                    // Quantize in transpose mode 1: Semitones
                    voltage[i] = voltage[i] + params[TRANSPOSE_PARAM + col].getValue() * 1.f / 12.f;
                    voltage[i] = Quantizer::quantize(clamp(voltage[i], -5.f, 5.f), scale[scene]);
                }
                if (params[TRANSPOSE_MODE_PARAM + col].getValue() == 2.f) {
                    // Quantize in transpose mode 2: Scale degrees
                    voltage[i] = Quantizer::quantize(clamp(voltage[i], -5.f, 5.f), scale[scene], (int) params[TRANSPOSE_PARAM + col].getValue());
                }
                shVoltage[col][i] = voltage[i];
            } else {
                // No S&H
                voltage[i] = shVoltage[col][i];
            }

            // Piano display
            if (params[VISUALIZE_PARAM + col].getValue() == 1.f) {
                 // Must be positive to work. The 0.01f is to fudge float math in transpose mode 2.
                float v = voltage[i] * 12.f + 60.01f;
                int n = (int) v % 12;
                litKeys[n] = true;
            }

            // Output!
            outputs[CV_OUTPUT + col].setVoltage(voltage[i], i);
        }

        outputs[CV_OUTPUT + col].setChannels( (sh) ? channels : shChannels[col]);
    }


    void updateLcd(const ProcessArgs& args){
        std::string text;

        // Reset after 3 seconds since the last interactive input was touched
        if (lcdLastInteraction < (3.f / LCDDIVIDER) ) {
            lcdLastInteraction += args.sampleTime;
            if(lcdLastInteraction >= (3.f / LCDDIVIDER) ) {
                if (lcdMode == INIT_MODE) {
                    // This module has 2 load messages
                    lcdMode = LOAD_MODE;
                    lcdLastInteraction = 0.f;
                } else {
                    lcdMode = READY_MODE;
                }
                lcdStatus.dirty = true;
            }
        }

        if (lcdMode == LOAD_MODE) {
            lcdStatus.text1 = " Q< Quack!";
        }

        if (lcdMode == READY_MODE) {
            lcdStatus.text1 = " Q<";
        }

        if (lcdMode == SCALE_MODE) {
            if(params[SCALE_PARAM].getValue() == 0.f) {
                text = "CHROMATIC";
            } else {
                text = Quantizer::keyLcdName((int)params[KEY_PARAM].getValue());
                text.append(" ");
                text.append(Quantizer::scaleLcdName((int)params[SCALE_PARAM].getValue()));
            }
            lcdStatus.text1 = text;
        }

        if (lcdMode == SCALING_MODE) {
            text = std::to_string((int) params[lastScalingKnobTouchedId].getValue());
            text.append("%");
            lcdStatus.text1 = text;
        }

        if (lcdMode == OFFSET_MODE) {
            text = std::to_string(params[lastOffsetKnobTouchedId].getValue());
            text.resize(5);
            text.append("V");
            lcdStatus.text1 = text;
        }

        if (lcdMode == TRANSPOSE_MODE) {
            text = std::to_string((int) params[lastTransposeKnobTouchedId].getValue());
            // Nasty hack that depends on NEVER changing the order or number of the params
            if (params[lastTransposeKnobTouchedId + 4].getValue() == 0.f) text.append(" Oct.");
            if (params[lastTransposeKnobTouchedId + 4].getValue() == 1.f) text.append(" St.");
            if (params[lastTransposeKnobTouchedId + 4].getValue() == 2.f) text.append(" S.D.");
            lcdStatus.text1 = text;
        }

        // Button operated are set to dirty while the mode shows, for simplicity of processing.
        if (lcdMode == TRANSPOSE_TYPE_MODE) {
            text = "";
            if (params[lastTransposeModeTouchedId].getValue() == 0.f) text = ("Octaves");
            if (params[lastTransposeModeTouchedId].getValue() == 1.f) text = ("Semitones");
            if (params[lastTransposeModeTouchedId].getValue() == 2.f) text = ("Scale Deg.");
            lcdStatus.text1 = text;
            lcdStatus.dirty = true;
        }

        if (lcdMode == SH_MODE) {
            lcdStatus.text1 = (params[lastShTouchedId].getValue() == 0.f) ? "Sample & H." : "Track  & H.";
            lcdStatus.dirty = true;
        }

        if (lcdMode == VISUALIZE_MODE) {
            lcdStatus.text1 = "<-Visualize";
            lcdStatus.dirty = true;
        }

    }


    void process(const ProcessArgs& args) override {
        if (processDivider.process()) {
            updateExpander();
            updateScene();
            updateScale();
            cleanLitKeys();
            processInputs();
            for(int i = 0; i < 4; i++) processQuantizerColumn(i);
            updateExternalOutput();
        }
        if (lcdDivider.process()) {
            updateLcd(args);
        }
        // Fixes MIDI-MAP turning off buttons
        params[SCENE_BUTTON_PARAM + scene].setValue(1.f);
    }

};



///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////



namespace QqqqWidgets {

// The LCD knobs
struct LcdKnob : W::Knob {
    void onDragMove(const event::DragMove& e) override {
        ParamQuantity* const paramQuantity = getParamQuantity();
        dynamic_cast<Qqqq*>(paramQuantity->module)->lcdLastInteraction = 0.f;
        dynamic_cast<Qqqq*>(paramQuantity->module)->lcdStatus.dirty = true;
        W::Knob::onDragMove(e);
    }
};
struct ScaleKnob : LcdKnob {
    ScaleKnob() {
        snap = true;
        LcdKnob();
    }
    void onDragMove(const event::DragMove& e) override {
        ParamQuantity* const paramQuantity = getParamQuantity();
        dynamic_cast<Qqqq*>(paramQuantity->module)->lcdMode = SCALE_MODE;
        LcdKnob::onDragMove(e);
    }
};
struct ScalingKnob : LcdKnob {
    void onDragMove(const event::DragMove& e) override {
        ParamQuantity* const paramQuantity = getParamQuantity();
        dynamic_cast<Qqqq*>(paramQuantity->module)->lcdMode = SCALING_MODE;
        dynamic_cast<Qqqq*>(paramQuantity->module)->lastScalingKnobTouchedId = paramQuantity->paramId;
        LcdKnob::onDragMove(e);
    }
};
struct OffsetKnob : LcdKnob {
    void onDragMove(const event::DragMove& e) override {
        ParamQuantity* const paramQuantity = getParamQuantity();
        dynamic_cast<Qqqq*>(paramQuantity->module)->lcdMode = OFFSET_MODE;
        dynamic_cast<Qqqq*>(paramQuantity->module)->lastOffsetKnobTouchedId = paramQuantity->paramId;
        LcdKnob::onDragMove(e);
    }
};
struct TransposeKnob : LcdKnob {
    TransposeKnob() {
        snap = true;
        LcdKnob();
    }
    void onDragMove(const event::DragMove& e) override {
        ParamQuantity* const paramQuantity = getParamQuantity();
        dynamic_cast<Qqqq*>(paramQuantity->module)->lcdMode = TRANSPOSE_MODE;
        dynamic_cast<Qqqq*>(paramQuantity->module)->lastTransposeKnobTouchedId = paramQuantity->paramId;
        LcdKnob::onDragMove(e);
    }
};
// The LCD buttons. They're not sending ongoing events so no point setting Lcd dirty from here.
struct TransposeButton : W::SmallButton {
    void onDragStart(const event::DragStart& e) override {
        ParamQuantity* const paramQuantity = getParamQuantity();
        dynamic_cast<Qqqq*>(paramQuantity->module)->lcdLastInteraction = 0.f;
        dynamic_cast<Qqqq*>(paramQuantity->module)->lcdMode = TRANSPOSE_TYPE_MODE;
        dynamic_cast<Qqqq*>(paramQuantity->module)->lastTransposeModeTouchedId = paramQuantity->paramId;
        W::SmallButton::onDragStart(e);
    }
};
struct ShButton : W::SmallButton {
    void onDragStart(const event::DragStart& e) override {
        ParamQuantity* const paramQuantity = getParamQuantity();
        dynamic_cast<Qqqq*>(paramQuantity->module)->lcdLastInteraction = 0.f;
        dynamic_cast<Qqqq*>(paramQuantity->module)->lcdMode = SH_MODE;
        dynamic_cast<Qqqq*>(paramQuantity->module)->lastShTouchedId = paramQuantity->paramId;
        W::SmallButton::onDragStart(e);
    }
};
struct VisualizeButton : W::ButtonPink {
    void onDragStart(const event::DragStart& e) override {
        ParamQuantity* const paramQuantity = getParamQuantity();
        dynamic_cast<Qqqq*>(paramQuantity->module)->lcdLastInteraction = 0.f;
        dynamic_cast<Qqqq*>(paramQuantity->module)->lcdMode = VISUALIZE_MODE;
        W::ButtonPink::onDragStart(e);
    }
};

// The piano display
// https://community.vcvrack.com/t/whats-the-best-way-to-implement-a-pushbutton-with-three-visual-states-but-only-two-user-controllable-states/10351/8?u=aria_salvatrice
struct PianoKey : W::LitSvgSwitchUnshadowed {
    bool lastPianoDisplay = false;
    bool currentPianoDisplay = false;
    int note = 0;

    void step() override {
        if (ParamQuantity* const paramQuantity = getParamQuantity()){
            currentPianoDisplay = dynamic_cast<Qqqq*>(paramQuantity->module)->litKeys[note];
            if (currentPianoDisplay == true && currentPianoDisplay != lastPianoDisplay) {
                lsw->setSvg(frames[2]);
                fb->dirty = true;
            }
            if (currentPianoDisplay == false && currentPianoDisplay != lastPianoDisplay) {
                int index = (int) std::round(paramQuantity->getValue() - paramQuantity->getMinValue());
                index = math::clamp(index, 0, (int) frames.size() - 1);
                if (index > 0) {
                    lsw->setSvg(frames[index]);
                } else {
                    lsw->hide();
                }
                fb->dirty = true; 
            }
            lastPianoDisplay = currentPianoDisplay;
        }
        W::LitSvgSwitchUnshadowed::step();
    }
};

// To represent an octave, only 6 distinct key graphics are required: C, E, F, B, White with notch on each side, Black
struct PianoWhiteKey : PianoKey {
    PianoWhiteKey() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/unlit-white.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/yellow-white.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/pink-white.svg")));
        PianoKey();
    }
};
struct PianoBlackKey : PianoKey {
    PianoBlackKey() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/unlit-black.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/yellow-black.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/pink-black.svg")));
        PianoKey();
    }
};
struct PianoC : PianoKey {
    PianoC() {
        note = 0;
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/unlit-C.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/yellow-C.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/pink-C.svg")));
        PianoKey();
    }
};
struct PianoCSharp : PianoBlackKey { PianoCSharp() { note = 1; PianoBlackKey();} };
struct PianoD : PianoWhiteKey { PianoD () { note = 2; PianoWhiteKey(); } };
struct PianoDSharp : PianoBlackKey { PianoDSharp() { note = 3; PianoBlackKey();} };
struct PianoE : PianoKey {
    PianoE() {
        note = 4;
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/unlit-E.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/yellow-E.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/pink-E.svg")));
        PianoKey();
    }
};
struct PianoF : PianoKey {
    PianoF() {
        note = 5;
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/unlit-F.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/yellow-F.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/pink-F.svg")));
        PianoKey();
    }
};
struct PianoFSharp : PianoBlackKey { PianoFSharp() { note = 6; PianoBlackKey();} };
struct PianoG : PianoWhiteKey { PianoG () { note = 7; PianoWhiteKey(); } };
struct PianoGSharp : PianoBlackKey { PianoGSharp() { note = 8; PianoBlackKey();} };
struct PianoA : PianoWhiteKey { PianoA () { note = 9; PianoWhiteKey(); } };
struct PianoASharp : PianoBlackKey { PianoASharp() { note = 10; PianoBlackKey();} };
struct PianoB : PianoKey {
    PianoB() {
        note = 11;
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/unlit-B.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/yellow-B.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/pink-B.svg")));
        PianoKey();
    }
};

// Keyboard
struct LeadSheetField : ui::TextField {
    Qqqq* module;
    LeadSheetField() {
        box.size.x = 100.f;
        placeholder = "C em A7 G7sus4 Eb G/D G7sus4 Cmaj7";
    }
    void onAction(const event::Action& e) override {
        module->importLeadSheet(rack::string::trim(text));
        TextField::onAction(e);
        getAncestorOfType<ui::MenuOverlay>()->requestDelete();
    }
};

struct RomanNumeralField : ui::TextField {
    Qqqq* module;
    RomanNumeralField() {
        box.size.x = 100.f;
        placeholder = "I V vim7 V bVI bIII bVII IV";
    }
    void onAction(const event::Action& e) override {
        module->importRomanNumeral(rack::string::trim(text));
        TextField::onAction(e);
        getAncestorOfType<ui::MenuOverlay>()->requestDelete();
    }
};

struct CopyPortableSequenceItem : MenuItem {
    Qqqq *module;
    void onAction(const event::Action &e) override {
        module->copyPortableSequence();
    }
};
struct PastePortableSequenceItem : MenuItem {
    Qqqq *module;
    void onAction(const event::Action &e) override {
        module->pastePortableSequence();
    }
};

struct PushButtonKeyboard : W::SvgSwitchUnshadowed {
    PushButtonKeyboard() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/keyboard-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/keyboard-on.svg")));
        momentary = true;
        W::SvgSwitchUnshadowed();
    }

    void onButton(const event::Button& e) override {
    	if (e.button != GLFW_MOUSE_BUTTON_LEFT) return; // Skip context menu

        ui::Menu* menu = createMenu();

        ParamQuantity* const paramQuantity = getParamQuantity();

        LeadSheetField* lsf = new LeadSheetField();
        lsf->module = dynamic_cast<Qqqq*>(paramQuantity->module);
        menu->addChild(createMenuLabel("Import chords (lead sheet notation):"));
        menu->addChild(lsf);
        menu->addChild(new MenuSeparator());

        RomanNumeralField* rnf = new RomanNumeralField();
        rnf->module = dynamic_cast<Qqqq*>(paramQuantity->module);
        menu->addChild(createMenuLabel("Import chords (roman numeral notation):"));
        menu->addChild(rnf);
        menu->addChild(new MenuSeparator());

        CopyPortableSequenceItem *copyPortableSequenceItem = createMenuItem<CopyPortableSequenceItem>("Copy Scenes as Portable Sequence");
        copyPortableSequenceItem->module = dynamic_cast<Qqqq*>(paramQuantity->module);
        menu->addChild(copyPortableSequenceItem);

        PastePortableSequenceItem *pastePortableSequenceItem = createMenuItem<PastePortableSequenceItem>("Paste Portable Sequence as Scenes");
        pastePortableSequenceItem->module = dynamic_cast<Qqqq*>(paramQuantity->module);
        menu->addChild(pastePortableSequenceItem);

        e.consume(this);
    }
};

// Scene buttons, we'll give them frames later.
// Automating it breaks the module browser so you know what, I'm not even gonna bother being clever about this.
struct CopyScenePortableSequenceItem : MenuItem {
    Qqqq *module;
    int slot;
    void onAction(const event::Action &e) override {
        module->copyScenePortableSequence(slot);
    }
};
struct PasteScenePortableSequenceItem : MenuItem {
    Qqqq *module;
    int slot;
    void onAction(const event::Action &e) override {
        module->pasteScenePortableSequence(slot);
    }
};

struct SceneButton : W::LitSvgSwitchUnshadowed {
    void onButton(const event::Button& e) override {
        if (e.button == GLFW_MOUSE_BUTTON_RIGHT) {
            ui::Menu* menu = createMenu();

            ParamQuantity* const paramQuantity = getParamQuantity();

            CopyScenePortableSequenceItem *copyScenePortableSequenceItem = new CopyScenePortableSequenceItem();
            copyScenePortableSequenceItem->text = "Copy Scene";
            copyScenePortableSequenceItem->slot = paramQuantity->paramId - Qqqq::SCENE_BUTTON_PARAM;
            copyScenePortableSequenceItem->module = dynamic_cast<Qqqq*>(paramQuantity->module);
            menu->addChild(copyScenePortableSequenceItem);

            PasteScenePortableSequenceItem *pasteScenePortableSequenceItem = new PasteScenePortableSequenceItem();
            pasteScenePortableSequenceItem->text = "Paste Scene";
            pasteScenePortableSequenceItem->slot = paramQuantity->paramId - Qqqq::SCENE_BUTTON_PARAM;
            pasteScenePortableSequenceItem->module = dynamic_cast<Qqqq*>(paramQuantity->module);
            menu->addChild(pasteScenePortableSequenceItem);

            e.consume(this);
        } else {
            W::LitSvgSwitchUnshadowed::onButton(e);
        }
    }
};
struct SceneButton01 : SceneButton {
    SceneButton01() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/numbered-buttons/01.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/numbered-buttons/01-lit.svg")));
    }
};
struct SceneButton02 : SceneButton {
    SceneButton02() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/numbered-buttons/02.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/numbered-buttons/02-lit.svg")));
    }
};
struct SceneButton03 : SceneButton {
    SceneButton03() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/numbered-buttons/03.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/numbered-buttons/03-lit.svg")));
    }
};
struct SceneButton04 : SceneButton {
    SceneButton04() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/numbered-buttons/04.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/numbered-buttons/04-lit.svg")));
    }
};
struct SceneButton05 : SceneButton {
    SceneButton05() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/numbered-buttons/05.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/numbered-buttons/05-lit.svg")));
    }
};
struct SceneButton06 : SceneButton {
    SceneButton06() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/numbered-buttons/06.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/numbered-buttons/06-lit.svg")));
    }
};
struct SceneButton07 : SceneButton {
    SceneButton07() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/numbered-buttons/07.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/numbered-buttons/07-lit.svg")));
    }
};
struct SceneButton08 : SceneButton {
    SceneButton08() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/numbered-buttons/08.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/numbered-buttons/08-lit.svg")));
    }
};
struct SceneButton09 : SceneButton {
    SceneButton09() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/numbered-buttons/09.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/numbered-buttons/09-lit.svg")));
    }
};
struct SceneButton10 : SceneButton {
    SceneButton10() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/numbered-buttons/10.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/numbered-buttons/10-lit.svg")));
    }
};
struct SceneButton11 : SceneButton {
    SceneButton11() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/numbered-buttons/11.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/numbered-buttons/11-lit.svg")));
    }
};
struct SceneButton12 : SceneButton {
    SceneButton12() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/numbered-buttons/12.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/numbered-buttons/12-lit.svg")));
    }
};
struct SceneButton13 : SceneButton {
    SceneButton13() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/numbered-buttons/13.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/numbered-buttons/13-lit.svg")));
    }
};
struct SceneButton14 : SceneButton {
    SceneButton14() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/numbered-buttons/14.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/numbered-buttons/14-lit.svg")));
    }
};
struct SceneButton15 : SceneButton {
    SceneButton15() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/numbered-buttons/15.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/numbered-buttons/15-lit.svg")));
    }
};
struct SceneButton16 : SceneButton {
    SceneButton16() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/numbered-buttons/16.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/numbered-buttons/16-lit.svg")));
    }
};



} // namespace QqqqWidgets




///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////


struct QqqqWidget : W::ModuleWidget {

    void drawScrews() {
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    }

    void drawPianoKeys(float xOffset, float yOffset, Qqqq* module) {
        // First we create the white keys only.
        addParam(createParam<QqqqWidgets::PianoC>(mm2px(Vec(xOffset, yOffset -  0.f)), module, Qqqq::NOTE_PARAM +  0)); // C
        addParam(createParam<QqqqWidgets::PianoD>(mm2px(Vec(xOffset, yOffset - 14.f)), module, Qqqq::NOTE_PARAM +  2)); // D
        addParam(createParam<QqqqWidgets::PianoE>(mm2px(Vec(xOffset, yOffset - 28.f)), module, Qqqq::NOTE_PARAM +  4)); // E
        addParam(createParam<QqqqWidgets::PianoF>(mm2px(Vec(xOffset, yOffset - 42.f)), module, Qqqq::NOTE_PARAM +  5)); // F
        addParam(createParam<QqqqWidgets::PianoG>(mm2px(Vec(xOffset, yOffset - 56.f)), module, Qqqq::NOTE_PARAM +  7)); // G
        addParam(createParam<QqqqWidgets::PianoA>(mm2px(Vec(xOffset, yOffset - 70.f)), module, Qqqq::NOTE_PARAM +  9)); // A
        addParam(createParam<QqqqWidgets::PianoB>(mm2px(Vec(xOffset, yOffset - 84.f)), module, Qqqq::NOTE_PARAM + 11)); // B
        // Then, the black keys, so they overlap the clickable area of the white keys, avoiding the need he need for a custom widget.
        addParam(createParam<QqqqWidgets::PianoCSharp>(mm2px(Vec(xOffset, yOffset -  5.f)), module, Qqqq::NOTE_PARAM +  1)); // C#
        addParam(createParam<QqqqWidgets::PianoDSharp>(mm2px(Vec(xOffset, yOffset - 19.f)), module, Qqqq::NOTE_PARAM +  3)); // D#
        addParam(createParam<QqqqWidgets::PianoFSharp>(mm2px(Vec(xOffset, yOffset - 47.f)), module, Qqqq::NOTE_PARAM +  6)); // F#
        addParam(createParam<QqqqWidgets::PianoGSharp>(mm2px(Vec(xOffset, yOffset - 61.f)), module, Qqqq::NOTE_PARAM +  8)); // G#
        addParam(createParam<QqqqWidgets::PianoASharp>(mm2px(Vec(xOffset, yOffset - 75.f)), module, Qqqq::NOTE_PARAM + 10)); // A#
    }

    void drawQuantizerColumn(float xOffset, float yOffset, Qqqq* module, int col) {
        addStaticInput(mm2px(Vec(xOffset + 0.f, yOffset + 0.f)), module, Qqqq::CV_INPUT + col);
        addParam(createParam<QqqqWidgets::ScalingKnob>(mm2px(Vec(xOffset + 0.f, yOffset + 10.f)), module, Qqqq::SCALING_PARAM + col));
        addParam(createParam<QqqqWidgets::OffsetKnob>(mm2px(Vec(xOffset + 0.f, yOffset + 20.f)), module, Qqqq::OFFSET_PARAM + col));
        addParam(createParam<QqqqWidgets::TransposeKnob>(mm2px(Vec(xOffset + 0.f, yOffset + 30.f)), module, Qqqq::TRANSPOSE_PARAM + col));

        addParam(createParam<QqqqWidgets::TransposeButton>(mm2px(Vec(xOffset + 3.5f, yOffset + 40.f)), module, Qqqq::TRANSPOSE_MODE_PARAM + col));
        addParam(createParam<QqqqWidgets::ShButton>(mm2px(Vec(xOffset + -0.5f, yOffset + 42.5f)), module, Qqqq::SH_MODE_PARAM + col));

        addStaticInput(mm2px(Vec(xOffset + 0.f, yOffset + 50.f)), module, Qqqq::SH_INPUT + col);
        addParam(createParam<QqqqWidgets::VisualizeButton>(mm2px(Vec(xOffset + 0.f, yOffset + 60.f)), module, Qqqq::VISUALIZE_PARAM + col));
        addStaticOutput(mm2px(Vec(xOffset + 0.f, yOffset + 70.f)), module, Qqqq::CV_OUTPUT + col);
    }

    void drawSceneSlots(float xOffset, float yOffset, Qqqq* module) {
        addParam(createParam<QqqqWidgets::SceneButton01>(mm2px(Vec(xOffset +  0.f, yOffset -  0.f)), module, Qqqq::SCENE_BUTTON_PARAM +  0));
        addParam(createParam<QqqqWidgets::SceneButton02>(mm2px(Vec(xOffset +  8.f, yOffset -  0.f)), module, Qqqq::SCENE_BUTTON_PARAM +  1));
        addParam(createParam<QqqqWidgets::SceneButton03>(mm2px(Vec(xOffset + 16.f, yOffset -  0.f)), module, Qqqq::SCENE_BUTTON_PARAM +  2));
        addParam(createParam<QqqqWidgets::SceneButton04>(mm2px(Vec(xOffset + 24.f, yOffset -  0.f)), module, Qqqq::SCENE_BUTTON_PARAM +  3));
        addParam(createParam<QqqqWidgets::SceneButton05>(mm2px(Vec(xOffset +  0.f, yOffset -  8.f)), module, Qqqq::SCENE_BUTTON_PARAM +  4));
        addParam(createParam<QqqqWidgets::SceneButton06>(mm2px(Vec(xOffset +  8.f, yOffset -  8.f)), module, Qqqq::SCENE_BUTTON_PARAM +  5));
        addParam(createParam<QqqqWidgets::SceneButton07>(mm2px(Vec(xOffset + 16.f, yOffset -  8.f)), module, Qqqq::SCENE_BUTTON_PARAM +  6));
        addParam(createParam<QqqqWidgets::SceneButton08>(mm2px(Vec(xOffset + 24.f, yOffset -  8.f)), module, Qqqq::SCENE_BUTTON_PARAM +  7));
        addParam(createParam<QqqqWidgets::SceneButton09>(mm2px(Vec(xOffset +  0.f, yOffset - 16.f)), module, Qqqq::SCENE_BUTTON_PARAM +  8));
        addParam(createParam<QqqqWidgets::SceneButton10>(mm2px(Vec(xOffset +  8.f, yOffset - 16.f)), module, Qqqq::SCENE_BUTTON_PARAM +  9));
        addParam(createParam<QqqqWidgets::SceneButton11>(mm2px(Vec(xOffset + 16.f, yOffset - 16.f)), module, Qqqq::SCENE_BUTTON_PARAM + 10));
        addParam(createParam<QqqqWidgets::SceneButton12>(mm2px(Vec(xOffset + 24.f, yOffset - 16.f)), module, Qqqq::SCENE_BUTTON_PARAM + 11));
        addParam(createParam<QqqqWidgets::SceneButton13>(mm2px(Vec(xOffset +  0.f, yOffset - 24.f)), module, Qqqq::SCENE_BUTTON_PARAM + 12));
        addParam(createParam<QqqqWidgets::SceneButton14>(mm2px(Vec(xOffset +  8.f, yOffset - 24.f)), module, Qqqq::SCENE_BUTTON_PARAM + 13));
        addParam(createParam<QqqqWidgets::SceneButton15>(mm2px(Vec(xOffset + 16.f, yOffset - 24.f)), module, Qqqq::SCENE_BUTTON_PARAM + 14));
        addParam(createParam<QqqqWidgets::SceneButton16>(mm2px(Vec(xOffset + 24.f, yOffset - 24.f)), module, Qqqq::SCENE_BUTTON_PARAM + 15));
    }

    QqqqWidget(Qqqq* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/faceplates/Qqqq.svg")));
        
        // Signature
        addChild(createWidget<W::Signature>(mm2px(Vec(65.f, 114.5f))));

        drawScrews();
        drawPianoKeys(4.7f, 102.8f, module);

        // LCD
        Lcd::LcdWidget<Qqqq> *lcd = new Lcd::LcdWidget<Qqqq>(module, " Q< Quack!");
        lcd->box.pos = mm2px(Vec(27.6f, 21.2f));
        addChild(lcd);

        // Scale, Key, External
        addParam(createParam<QqqqWidgets::ScaleKnob>(mm2px(Vec(25.f, 29.f)), module, Qqqq::KEY_PARAM));
        addParam(createParam<QqqqWidgets::ScaleKnob>(mm2px(Vec(35.f, 29.f)), module, Qqqq::SCALE_PARAM));
        addStaticInput(mm2px(Vec(45.f, 29.f)), module, Qqqq::EXT_SCALE_INPUT);
        addStaticOutput(mm2px(Vec(55.f, 29.f)), module, Qqqq::EXT_SCALE_OUTPUT);

        // Scene programmer. Offset by 0.1mm because it looks better that way
        drawSceneSlots(67.6f, 42.5f, module);
        addStaticInput(mm2px(Vec(84.f, 53.f)), module, Qqqq::SCENE_INPUT);

        // Keyboard inputs
        addParam(createParam<QqqqWidgets::PushButtonKeyboard>(mm2px(Vec(83.f, 66.5f)), module, Qqqq::KEYBOARD_INPUT_PARAM));

        // The quantizer columns
        drawQuantizerColumn(25.f, 43.f, module, 0);
        drawQuantizerColumn(35.f, 43.f, module, 1);
        drawQuantizerColumn(45.f, 43.f, module, 2);
        drawQuantizerColumn(55.f, 43.f, module, 3);

        // Expander lights (right is 3.5mm from edge)
        addChild(createLight<W::StatusLightInput>(mm2px(Vec(1.4, 125.2)), module, Qqqq::EXPANDER_IN_LIGHT));
        addChild(createLight<W::StatusLightOutput>(mm2px(Vec(98.1, 125.2)), module, Qqqq::EXPANDER_OUT_LIGHT));
    }


    struct SceneStandardSelectionConfigItem : MenuItem {
        Qqqq *module;
        void onAction(const event::Action &e) override {
            module->sceneTrigSelection = false;
        }
    };

    struct SceneTrigSelectionConfigItem : MenuItem {
        Qqqq *module;
        void onAction(const event::Action &e) override {
            module->sceneTrigSelection = true;
        }
    };


    void appendContextMenu(ui::Menu *menu) override {	
        Qqqq *module = dynamic_cast<Qqqq*>(this->module);
        assert(module);

        menu->addChild(new MenuSeparator());

        SceneStandardSelectionConfigItem *sceneStandardSelectionConfigItem = createMenuItem<SceneStandardSelectionConfigItem>("Select Scenes with 0V~10V");
        sceneStandardSelectionConfigItem->module = module;
        sceneStandardSelectionConfigItem->rightText += (module->sceneTrigSelection) ? "" : "✔";
        menu->addChild(sceneStandardSelectionConfigItem);

        SceneTrigSelectionConfigItem *sceneTrigSelectionConfigItem = createMenuItem<SceneTrigSelectionConfigItem>("Advance Scenes with trigs");
        sceneTrigSelectionConfigItem->module = module;
        sceneTrigSelectionConfigItem->rightText += (module->sceneTrigSelection) ? "✔" : "";
        menu->addChild(sceneTrigSelectionConfigItem);
    }


};




///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////




struct QuackWidget : W::ModuleWidget {

    void drawScrews() {
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    }

    void drawPianoKeys(float xOffset, float yOffset, Qqqq* module) {
        // First we create the white keys only.
        addParam(createParam<QqqqWidgets::PianoC>(mm2px(Vec(xOffset, yOffset -  0.f)), module, Qqqq::NOTE_PARAM +  0)); // C
        addParam(createParam<QqqqWidgets::PianoD>(mm2px(Vec(xOffset, yOffset - 14.f)), module, Qqqq::NOTE_PARAM +  2)); // D
        addParam(createParam<QqqqWidgets::PianoE>(mm2px(Vec(xOffset, yOffset - 28.f)), module, Qqqq::NOTE_PARAM +  4)); // E
        addParam(createParam<QqqqWidgets::PianoF>(mm2px(Vec(xOffset, yOffset - 42.f)), module, Qqqq::NOTE_PARAM +  5)); // F
        addParam(createParam<QqqqWidgets::PianoG>(mm2px(Vec(xOffset, yOffset - 56.f)), module, Qqqq::NOTE_PARAM +  7)); // G
        addParam(createParam<QqqqWidgets::PianoA>(mm2px(Vec(xOffset, yOffset - 70.f)), module, Qqqq::NOTE_PARAM +  9)); // A
        addParam(createParam<QqqqWidgets::PianoB>(mm2px(Vec(xOffset, yOffset - 84.f)), module, Qqqq::NOTE_PARAM + 11)); // B
        // Then, the black keys, so they overlap the clickable area of the white keys, avoiding the need he need for a custom widget.
        addParam(createParam<QqqqWidgets::PianoCSharp>(mm2px(Vec(xOffset, yOffset -  5.f)), module, Qqqq::NOTE_PARAM +  1)); // C#
        addParam(createParam<QqqqWidgets::PianoDSharp>(mm2px(Vec(xOffset, yOffset - 19.f)), module, Qqqq::NOTE_PARAM +  3)); // D#
        addParam(createParam<QqqqWidgets::PianoFSharp>(mm2px(Vec(xOffset, yOffset - 47.f)), module, Qqqq::NOTE_PARAM +  6)); // F#
        addParam(createParam<QqqqWidgets::PianoGSharp>(mm2px(Vec(xOffset, yOffset - 61.f)), module, Qqqq::NOTE_PARAM +  8)); // G#
        addParam(createParam<QqqqWidgets::PianoASharp>(mm2px(Vec(xOffset, yOffset - 75.f)), module, Qqqq::NOTE_PARAM + 10)); // A#
    }

    void drawQuantizerColumn(float xOffset, float yOffset, Qqqq* module, int col) {
        addStaticInput(mm2px(Vec(xOffset + 0.f, yOffset + 0.f)), module, Qqqq::CV_INPUT + col);
        addParam(createParam<QqqqWidgets::ScalingKnob>(mm2px(Vec(xOffset + 0.f, yOffset + 10.f)), module, Qqqq::SCALING_PARAM + col));
        addParam(createParam<QqqqWidgets::OffsetKnob>(mm2px(Vec(xOffset + 0.f, yOffset + 20.f)), module, Qqqq::OFFSET_PARAM + col));
        addParam(createParam<QqqqWidgets::TransposeKnob>(mm2px(Vec(xOffset + 0.f, yOffset + 30.f)), module, Qqqq::TRANSPOSE_PARAM + col));

        addParam(createParam<QqqqWidgets::TransposeButton>(mm2px(Vec(xOffset + 3.5f, yOffset + 40.f)), module, Qqqq::TRANSPOSE_MODE_PARAM + col));
        addParam(createParam<QqqqWidgets::ShButton>(mm2px(Vec(xOffset + -0.5f, yOffset + 42.5f)), module, Qqqq::SH_MODE_PARAM + col));

        addStaticInput(mm2px(Vec(xOffset + 0.f, yOffset + 50.f)), module, Qqqq::SH_INPUT + col);
        addStaticOutput(mm2px(Vec(xOffset + 0.f, yOffset + 60.f)), module, Qqqq::CV_OUTPUT + col);
    }

    QuackWidget(Qqqq* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/faceplates/Quack.svg")));
        
        // Signature
        addChild(createWidget<W::Signature>(mm2px(Vec(20.0f, 114.5f))));

        drawScrews();
        drawPianoKeys(1.7f, 102.8f, module);

        // Scale, Key, External
        addParam(createParam<QqqqWidgets::ScaleKnob>(mm2px(Vec(18.1f, 18.f)), module, Qqqq::KEY_PARAM));
        addParam(createParam<QqqqWidgets::ScaleKnob>(mm2px(Vec(26.4f, 18.f)), module, Qqqq::SCALE_PARAM));
        addStaticInput(mm2px(Vec(18.1f, 31.f)), module, Qqqq::EXT_SCALE_INPUT);
        addStaticOutput(mm2px(Vec(26.4f, 31.f)), module, Qqqq::EXT_SCALE_OUTPUT);

        // The quantizer column
        drawQuantizerColumn(22.f, 43.f, module, 0);

        // Expander lights (right is 3.5mm from edge)
        addChild(createLight<W::StatusLightInput>(mm2px(Vec(1.4, 125.2)), module, Qqqq::EXPANDER_IN_LIGHT));
        addChild(createLight<W::StatusLightOutput>(mm2px(Vec(32.06, 125.2)), module, Qqqq::EXPANDER_OUT_LIGHT));
    }
};




///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////




struct QWidget : W::ModuleWidget {

    void drawScrews() {
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    }

    // No visualize button in this version
    void drawQuantizerColumn(float xOffset, float yOffset, Qqqq* module, int col) {
        addStaticInput(mm2px(Vec(xOffset + 0.f, yOffset + 0.f)), module, Qqqq::CV_INPUT + col);
        addParam(createParam<QqqqWidgets::ScalingKnob>(mm2px(Vec(xOffset + 0.f, yOffset + 10.f)), module, Qqqq::SCALING_PARAM + col));
        addParam(createParam<QqqqWidgets::OffsetKnob>(mm2px(Vec(xOffset + 0.f, yOffset + 20.f)), module, Qqqq::OFFSET_PARAM + col));
        addParam(createParam<QqqqWidgets::TransposeKnob>(mm2px(Vec(xOffset + 0.f, yOffset + 30.f)), module, Qqqq::TRANSPOSE_PARAM + col));

        addParam(createParam<QqqqWidgets::TransposeButton>(mm2px(Vec(xOffset + 3.5f, yOffset + 40.f)), module, Qqqq::TRANSPOSE_MODE_PARAM + col));
        addParam(createParam<QqqqWidgets::ShButton>(mm2px(Vec(xOffset + -0.5f, yOffset + 42.5f)), module, Qqqq::SH_MODE_PARAM + col));

        addStaticInput(mm2px(Vec(xOffset + 0.f, yOffset + 50.f)), module, Qqqq::SH_INPUT + col);
        addStaticOutput(mm2px(Vec(xOffset + 0.f, yOffset + 60.f)), module, Qqqq::CV_OUTPUT + col);
    }

    QWidget(Qqqq* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/faceplates/Q.svg")));
        
        // Signature
        addChild(createWidget<W::Signature>(mm2px(Vec(1.0f, 114.5f))));

        drawScrews();

        // External
        addStaticInput(mm2px(Vec(3.52f, 29.f)), module, Qqqq::EXT_SCALE_INPUT);

        // Quantizer column
        drawQuantizerColumn(3.52f, 43.f, module, 0);

        // Expander lights (right is 3.5mm from edge)
        addChild(createLight<W::StatusLightInput>(mm2px(Vec(1.4, 125.2)), module, Qqqq::EXPANDER_IN_LIGHT));
        addChild(createLight<W::StatusLightOutput>(mm2px(Vec(11.74, 125.2)), module, Qqqq::EXPANDER_OUT_LIGHT));
    }
};

} // namespace Qqqq

Model* modelQqqq = createModel<Qqqq::Qqqq, Qqqq::QqqqWidget>("Qqqq");
Model* modelQuack = createModel<Qqqq::Qqqq, Qqqq::QuackWidget>("Quack");
Model* modelQ = createModel<Qqqq::Qqqq, Qqqq::QWidget>("Q");
