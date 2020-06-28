#include "plugin.hpp"
#include "quantizer.hpp"
#include "lcd.hpp"
#include "javascript.hpp"
#include "javascript-libraries.hpp"

/* Quatherina's Quality Quad Quantizer, Quack, Q<
   All three modules are the same, with different widgets.
*/ 

enum LcdModes {
    INIT_MODE,
    SCALE_MODE,
    SCALING_MODE,
    OFFSET_MODE,
    TRANSPOSE_MODE,
    SH_MODE,
    TRANSPOSETYPE_MODE,
};

// Nope, not gonna give you a placebo "HIGH CPU" right click option unless people
// raise valid complaints about this divider. 
const int PROCESSDIVIDER = 32;

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
        PASTE_CLIPBOARD_PARAM,
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
        NUM_LIGHTS
    };

    bool lastExtInConnected = false;
    bool sceneChanged = false;
    bool highCpu = false;
    int lcdMode = INIT_MODE;
    int scene = 0;
    int lastScene = 0;
    float lcdLastInteraction = 0.f;
    float lastKeyKnob = 0.f;
    float lastScaleKnob = 2.f;
    std::array<std::array<bool, 12>, 16> scale;
    std::array<bool, 12> lastExternalScale;
    std::array<bool, 12> litKeys;
    std::array<std::array<float, 16>, 4> inputVoltage;
    std::array<std::array<float, 16>, 4> shVoltage;
    std::array<int, 4> inputChannels;
    std::array<int, 4> shChannels;
    Lcd::LcdStatus lcdStatus;
    dsp::ClockDivider processDivider;
    dsp::SchmittTrigger shTrigger[4];
    
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
        for (int i = 0; i < 4; i++){
            configParam(SCALING_PARAM + i, -100.f, 300.f, 100.f, "Scaling", "%");
            configParam(OFFSET_PARAM + i, -10.f, 10.f, 0.f, "Offset", "V");
            configParam(TRANSPOSE_PARAM + i, -12.f, 12.f, 0.f, "Transpose");
            configParam(TRANSPOSE_MODE_PARAM + i, 0.f, 2.f, 0.f, "Transpose Mode");
            configParam(SH_MODE_PARAM + i, 0.f, 1.f, 0.f, "S&H / T&H Toggle");
            configParam(VISUALIZE_PARAM + i, 0.f, 1.f, 0.f, "Visualize on Piano");
        }
        configParam(SCENE_BUTTON_PARAM, 0.f, 1.f, 0.f, "Scene #1");
        for (int i = 1; i < 16; i++) configParam(SCENE_BUTTON_PARAM + i, 0.f, 1.f, 0.f, "Scene #" + std::to_string(i + 1));
        processDivider.setDivision(PROCESSDIVIDER);
        lcdMode = INIT_MODE;
        lcdLastInteraction = 0.f;
        lcdStatus.lcdText1 = " Q< quack~";
        lcdStatus.lcdPage = Lcd::TEXT1_PAGE;
        // Initialize
        for (int i = 0; i < 16; i++) { for (int j = 0; j < 12; j++) { scale[i][j] = false; }}
        // C Minor in first scene
        scale[0][0] = true; scale[0][2] = true; scale[0][3] = true; scale[0][5] = true; scale[0][7] = true; scale[0][8] = true; scale[0][10] = true;
    }

    // FIXME: JSON!
    // FIXME: On Reset
    // FIXME: Randomize

    // Sets the scene. The CV input overrides the buttons.
    void updateScene() {
        if (inputs[SCENE_INPUT].isConnected()) {
            scene = (int) rescale(inputs[SCENE_INPUT].getVoltageSum(), 0.f, 10.f, 0.f, 15.2f);
            if (scene != lastScene) sceneChanged = true;
            for (int i = 0; i < 16; i++) params[SCENE_BUTTON_PARAM + i].setValue( (i == scene) ? 1.f : 0.f );
        } else {
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
            // You shouldn't be able to turn off the current scene
            if (params[SCENE_BUTTON_PARAM + scene].getValue() == 0.f) params[SCENE_BUTTON_PARAM + scene].setValue(1.f);
        }
        lastScene = scene;
    }


    // Update the piano display to match the state of the internal scale if necessary
    void scaleToPiano() {
        for (int i = 0; i < 12; i++) {
            params[NOTE_PARAM + i].setValue((scale[scene][i]) ? 1.f : 0.f);
        }
    }

    // Update the internal scale to match the state of the piano display
    void pianoToScale() {
        for (int i = 0; i < 12; i++) scale[scene][i] = (params[NOTE_PARAM + i].getValue() == 1.f)  ? true : false;
    }


    // The last control touched always has the last word.
    void updateScale() {
        // Scene: has it changed?
        if (sceneChanged) {
            scaleToPiano();
            sceneChanged = false;
        }

        // FIXME: Expander: has it sent something?

        // External scale: was it just connected?
        if (!lastExtInConnected && inputs[EXT_SCALE_INPUT].isConnected()) {
            // After it's connected, we want to wait one more cycle to give it time to sync with slow inputs.
            for (int i = 0; i < 12; i++){
                scale[scene][i] = (inputs[EXT_SCALE_INPUT].getVoltage(i) > 0.f) ? true : false;
            } 
            scaleToPiano();
        }

        // External scale: has it changed?
        std::array<bool, 12> currentExternalScale;
        if (inputs[EXT_SCALE_INPUT].isConnected()) {
            for (int i = 0; i < 12; i++) currentExternalScale[i] = (inputs[EXT_SCALE_INPUT].getVoltage(i) > 0.f) ? true : false;
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
            for (int i = 0; i < 12; i++) outputs[EXT_SCALE_OUTPUT].setVoltage( (scale[scene][i]) ? 10.f : 0.f, i);
            outputs[EXT_SCALE_OUTPUT].setChannels(12);
        }
    }

    void cleanLitKeys() {
        for (int i =  0; i < 12; i++) litKeys[i] = false; 
    }

    // When there is no CV input, use the column to the left instead.
    void processInputs() {
        inputChannels[0] = inputs[CV_INPUT + 0].getChannels();
        for (int i = 0; i < inputChannels[0]; i++) inputVoltage[0][i] = inputs[CV_INPUT + 0].getVoltage(i);

        for (int i = 1; i < 4; i++) {
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
                // Quantize in transpose mode 0: Octaves
                if (params[TRANSPOSE_MODE_PARAM + col].getValue() == 0.f) {
                    voltage[i] = Quantizer::quantize(voltage[i], scale[scene]);
                    voltage[i] = voltage[i] + params[TRANSPOSE_PARAM + col].getValue();
                    voltage[i] = clamp(voltage[i], -10.f, 10.f);
                }
                // Quantize in transpose mode 1: Semitones
                if (params[TRANSPOSE_MODE_PARAM + col].getValue() == 1.f) {
                    voltage[i] = voltage[i] + params[TRANSPOSE_PARAM + col].getValue() * 1.f / 12.f;
                    voltage[i] = Quantizer::quantize(voltage[i], scale[scene]);
                }
                // Quantize in transpose mode 2: Scale degrees
                // FIXME: This one gonna be hard!!!!!
                if (params[TRANSPOSE_MODE_PARAM + col].getValue() == 2.f) {
                    voltage[i] = Quantizer::quantize(voltage[i], scale[scene]);
                }
                shVoltage[col][i] = voltage[i];
            } else {
                // No S&H
                voltage[i] = shVoltage[col][i];
            }

            // FIXME: S&H

            // Piano display
            if (params[VISUALIZE_PARAM + col].getValue() == 1.f) {
                float v = voltage[i] * 12.f + 60.f; // Must be positive to work
                int n = (int) v % 12;
                litKeys[n] = true;
            }

            // Output
            outputs[CV_OUTPUT + col].setVoltage(voltage[i], i);
        }

        outputs[CV_OUTPUT + col].setChannels( (sh) ? channels : shChannels[col]);
    }

    void process(const ProcessArgs& args) override {
        if (processDivider.process()) {
            updateScene();
            updateScale();
            cleanLitKeys();
            processInputs();
            for(int i = 0; i < 4; i++) processQuantizerColumn(i);
            updateExternalOutput();
        }
    }

};



///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////



namespace QqqqWidgets {

// The LCD knobs
struct LcdKnob : AriaKnob820 {
    void onDragMove(const event::DragMove& e) override {
         dynamic_cast<Qqqq*>(paramQuantity->module)->lcdLastInteraction = 0.f;
         dynamic_cast<Qqqq*>(paramQuantity->module)->lcdStatus.lcdDirty = true;
        AriaKnob820::onDragMove(e);
    }
};
struct ScaleKnob : LcdKnob {
    ScaleKnob() {
        snap = true;
        LcdKnob();
    }
    void onDragMove(const event::DragMove& e) override {
        dynamic_cast<Qqqq*>(paramQuantity->module)->lcdMode = SCALE_MODE;
        LcdKnob::onDragMove(e);
    }
};
struct TransposeKnob : LcdKnob {
    TransposeKnob() {
        snap = true;
        LcdKnob();
    }
    void onDragMove(const event::DragMove& e) override {
        dynamic_cast<Qqqq*>(paramQuantity->module)->lcdMode = TRANSPOSE_MODE;
        LcdKnob::onDragMove(e);
    }
};


// The piano display
// https://community.vcvrack.com/t/whats-the-best-way-to-implement-a-pushbutton-with-three-visual-states-but-only-two-user-controllable-states/10351/8?u=aria_salvatrice
struct PianoKey : SvgSwitchUnshadowed {
    bool lastPianoDisplay = false;
    bool currentPianoDisplay = false;
    int note = 0;

    void step() override {
        if (paramQuantity){
            currentPianoDisplay = dynamic_cast<Qqqq*>(paramQuantity->module)->litKeys[note];
            if (currentPianoDisplay == true && currentPianoDisplay != lastPianoDisplay) {
                sw->setSvg(frames[2]);
                fb->dirty = true;
            }
            if (currentPianoDisplay == false && currentPianoDisplay != lastPianoDisplay) {
                int index = (int) std::round(paramQuantity->getValue() - paramQuantity->getMinValue());
                index = math::clamp(index, 0, (int) frames.size() - 1);
                sw->setSvg(frames[index]);
                fb->dirty = true; 
            }
            lastPianoDisplay = currentPianoDisplay;
        }
        SvgSwitchUnshadowed::step();
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

// Keyboard, clipboard
struct PushButtonKeyboard : SvgSwitchUnshadowed {
    PushButtonKeyboard() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/button-keyboard.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/button-keyboard-pressed.svg")));
        momentary = true;
        SvgSwitchUnshadowed();
    }
};
struct PushButtonClipboard : SvgSwitchUnshadowed {
    PushButtonClipboard() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/button-clipboard.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/button-clipboard-pressed.svg")));
        momentary = true;
        SvgSwitchUnshadowed();
    }
};

// Scene buttons, we'll give them frames later.
struct SceneButton : SvgSwitchUnshadowed {
    SceneButton() {
        SvgSwitch();
    }
};

} // namespace QqqqWidgets




///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////




struct QqqqWidget : ModuleWidget {

    void drawScrews() {
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
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
        addInput(createInput<AriaJackIn>(mm2px(Vec(xOffset + 0.f, yOffset + 0.f)), module, Qqqq::CV_INPUT + col));
        addParam(createParam<AriaKnob820>(mm2px(Vec(xOffset + 0.f, yOffset + 10.f)), module, Qqqq::SCALING_PARAM + col));
        addParam(createParam<AriaKnob820>(mm2px(Vec(xOffset + 0.f, yOffset + 20.f)), module, Qqqq::OFFSET_PARAM + col));
        addParam(createParam<QqqqWidgets::TransposeKnob>(mm2px(Vec(xOffset + 0.f, yOffset + 30.f)), module, Qqqq::TRANSPOSE_PARAM + col));

        addParam(createParam<AriaPushButton500>(mm2px(Vec(xOffset + 3.5f, yOffset + 40.f)), module, Qqqq::TRANSPOSE_MODE_PARAM + col));
        addParam(createParam<AriaPushButton500>(mm2px(Vec(xOffset + -0.5f, yOffset + 42.5f)), module, Qqqq::SH_MODE_PARAM + col));

        addInput(createInput<AriaJackIn>(mm2px(Vec(xOffset + 0.f, yOffset + 50.f)), module, Qqqq::SH_INPUT + col));
        addParam(createParam<AriaPushButton820Pink>(mm2px(Vec(xOffset + 0.f, yOffset + 60.f)), module, Qqqq::VISUALIZE_PARAM + col));
        addOutput(createOutput<AriaJackOut>(mm2px(Vec(xOffset + 0.f, yOffset + 70.f)), module, Qqqq::CV_OUTPUT + col));
    }

    void drawSceneSlots(float xOffset, float yOffset, Qqqq* module) {
        int i = 0;
        std::string scene;
        for (int y = 0; y < 4; y++){
            for (int x = 0; x < 4; x++){
                if (i < 9) {
                    scene = "0" + std::to_string(i + 1);
                } else {
                    scene = std::to_string(i + 1);
                }
                QqqqWidgets::SceneButton* o = new QqqqWidgets::SceneButton;
                o->box.pos = mm2px(Vec(xOffset + x * 8.f, yOffset - y * 8.f));
                if (module) {
                    o->paramQuantity = module->paramQuantities[Qqqq::SCENE_BUTTON_PARAM + i];
                    o->addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/numbered-buttons/" + scene + ".svg")));
                    o->addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/numbered-buttons/" + scene + "-lit.svg")));
                }
                addParam(o);
                i++;
            }
        }
    }

    QqqqWidget(Qqqq* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Qqqq.svg")));
        
        // Signature
        addChild(createWidget<AriaSignature>(mm2px(Vec(65.f, 114.5f))));

        drawScrews();
        drawPianoKeys(4.7f, 102.8f, module);

        // The LCD will go around here
        addChild(Lcd::createLcd<Qqqq>(mm2px(Vec(27.6f, 21.2f)), module));

        // Scale, Key, External
        addParam(createParam<QqqqWidgets::ScaleKnob>(mm2px(Vec(25.f, 29.f)), module, Qqqq::SCALE_PARAM));
        addParam(createParam<QqqqWidgets::ScaleKnob>(mm2px(Vec(35.f, 29.f)), module, Qqqq::KEY_PARAM));
        addInput(createInput<AriaJackIn>(mm2px(Vec(45.f, 29.f)), module, Qqqq::EXT_SCALE_INPUT));
        addOutput(createOutput<AriaJackOut>(mm2px(Vec(55.f, 29.f)), module, Qqqq::EXT_SCALE_OUTPUT));

        // Scene programmer. Offset by 0.1mm because it looks better that way
        drawSceneSlots(67.6f, 42.5f, module);
        addInput(createInput<AriaJackIn>(mm2px(Vec(84.f, 53.f)), module, Qqqq::SCENE_INPUT));

        // Keyboard/Clipboard inputs
        addParam(createParam<QqqqWidgets::PushButtonKeyboard>(mm2px(Vec(83.f, 64.f)), module, Qqqq::KEYBOARD_INPUT_PARAM));
        addParam(createParam<QqqqWidgets::PushButtonClipboard>(mm2px(Vec(83.f, 73.f)), module, Qqqq::PASTE_CLIPBOARD_PARAM));

        // The quantizer columns
        drawQuantizerColumn(25.f, 43.f, module, 0);
        drawQuantizerColumn(35.f, 43.f, module, 1);
        drawQuantizerColumn(45.f, 43.f, module, 2);
        drawQuantizerColumn(55.f, 43.f, module, 3);
    }
};

Model* modelQqqq = createModel<Qqqq, QqqqWidget>("Qqqq");
Model* modelQuack = createModel<Qqqq, QqqqWidget>("Quack");
Model* modelQ = createModel<Qqqq, QqqqWidget>("Q");