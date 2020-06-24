#include "plugin.hpp"
#include "quantizer.hpp"
#include "lcd.hpp"
#include "javascript.hpp"
#include "javascript-libraries.hpp"

/* Quatherine's Quality Quad Quantizer, Quack, Q<
   All three modules are the same, with different widgets.
*/ 

enum LcdModes {
    INIT_MODE,
    SCALE_MODE
};

struct Qqqq : Module {
    enum ParamIds {
        ENUMS(NOTE_PARAM, 12),
        ENUMS(SCALING_PARAM, 4),
        ENUMS(OFFSET_PARAM, 4),
        ENUMS(TRANSPOSE_PARAM, 4),
        ENUMS(TRANSPOSE_MODE_PARAM, 4),
        ENUMS(SHTH_MODE_PARAM, 4),
        KEY_PARAM,
        SCALE_PARAM,
        SLOT_PARAM,
        PASTE_CLIPBOARD_PARAM,
        KEYBOARD_INPUT_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        ENUMS(CV_INPUT, 4),
        ENUMS(SCALING_INPUT, 4),
        ENUMS(OFFSET_INPUT, 4),
        ENUMS(TRANSPOSE_INPUT, 4),
        ENUMS(SHTH_INPUT, 4),
        EXT_SCALE_INPUT,
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

    bool pianoClicked = false;
    int lcdMode = INIT_MODE;
    float lcdLastInteraction = 0.f;
    std::array<bool, 12> scale;
    Lcd::LcdStatus lcdStatus;
    
    Qqqq() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(NOTE_PARAM +  0, 0.f, 2.f, 0.f, "C");
        configParam(NOTE_PARAM +  1, 0.f, 2.f, 0.f, "C#");
        configParam(NOTE_PARAM +  2, 0.f, 2.f, 0.f, "D");
        configParam(NOTE_PARAM +  3, 0.f, 2.f, 0.f, "D#");
        configParam(NOTE_PARAM +  4, 0.f, 2.f, 0.f, "E");
        configParam(NOTE_PARAM +  5, 0.f, 2.f, 0.f, "F");
        configParam(NOTE_PARAM +  6, 0.f, 2.f, 0.f, "F#");
        configParam(NOTE_PARAM +  7, 0.f, 2.f, 0.f, "G");
        configParam(NOTE_PARAM +  8, 0.f, 2.f, 0.f, "G#");
        configParam(NOTE_PARAM +  9, 0.f, 2.f, 0.f, "A");
        configParam(NOTE_PARAM + 10, 0.f, 2.f, 0.f, "A#");
        configParam(NOTE_PARAM + 11, 0.f, 2.f, 0.f, "B");
        configParam(KEY_PARAM, 0.f, 11.f, 0.f, "Key");
        configParam(SCALE_PARAM, 0.f, (float) Quantizer::NUM_SCALES - 1, 0.f, "Scale");
    }

    ~Qqqq(){
    }

    void updateScale(const ProcessArgs& args) {
        if (pianoClicked) {
            for (int i = 0; i < 12; i++) scale[i] = (params[NOTE_PARAM + i].getValue() == 1.f) ? true : false;
            pianoClicked = false;
        }
        
    }

    void process(const ProcessArgs& args) override {
        updateScale(args);
    }
};





///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////




struct AriaKnob820Scale : AriaKnob820 {
    Qqqq *module;

    AriaKnob820Scale(Qqqq* module) {
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



// Piano keys have three states, but users can only manually toggle two.
struct PianoKey : SvgSwitchUnshadowed {
    Qqqq* module;

    PianoKey() {
        this-> module = module;
        SvgSwitchUnshadowed();
    }

    void onDragMove(const event::DragMove& e) override {
        module->pianoClicked = true;
        SvgSwitchUnshadowed::onDragMove(e);
    }
};

// To represent the piano, only 6 distinct key graphics are required: C, E, F, B, White with notch on each side, Black
struct AriaPianoC : PianoKey {
    AriaPianoC() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/unlit-C.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/yellow-C.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/pink-C.svg")));
    }
};
struct AriaPianoE : PianoKey {
    AriaPianoE() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/unlit-E.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/yellow-E.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/pink-E.svg")));
    }
};
struct AriaPianoF : PianoKey {
    AriaPianoF() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/unlit-F.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/yellow-F.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/pink-F.svg")));
    }
};
struct AriaPianoB : PianoKey {
    AriaPianoB() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/unlit-B.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/yellow-B.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/pink-B.svg")));
    }
};
struct AriaPianoWhite : PianoKey {
    AriaPianoWhite() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/unlit-white.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/yellow-white.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/pink-white.svg")));
    }
};
struct AriaPianoBlack : PianoKey {
    AriaPianoBlack() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/unlit-black.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/yellow-black.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/pink-black.svg")));
    }
};


struct QqqqWidget : ModuleWidget {

    void drawScrews() {
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    }

    void drawPianoKeys(float pianoXoffset, float pianoYoffset, Qqqq* module) {
        // First we create the white keys only.
        addParam(createParam<AriaPianoC>(    mm2px(Vec(pianoXoffset, pianoYoffset -  0.f)), module, Qqqq::NOTE_PARAM +  0)); // C
        addParam(createParam<AriaPianoWhite>(mm2px(Vec(pianoXoffset, pianoYoffset - 14.f)), module, Qqqq::NOTE_PARAM +  2)); // D
        addParam(createParam<AriaPianoE>(    mm2px(Vec(pianoXoffset, pianoYoffset - 28.f)), module, Qqqq::NOTE_PARAM +  4)); // E
        addParam(createParam<AriaPianoF>(    mm2px(Vec(pianoXoffset, pianoYoffset - 42.f)), module, Qqqq::NOTE_PARAM +  5)); // F
        addParam(createParam<AriaPianoWhite>(mm2px(Vec(pianoXoffset, pianoYoffset - 56.f)), module, Qqqq::NOTE_PARAM +  7)); // G
        addParam(createParam<AriaPianoWhite>(mm2px(Vec(pianoXoffset, pianoYoffset - 70.f)), module, Qqqq::NOTE_PARAM +  9)); // A
        addParam(createParam<AriaPianoB>(    mm2px(Vec(pianoXoffset, pianoYoffset - 84.f)), module, Qqqq::NOTE_PARAM + 11)); // B
        // Then, the black keys, so they overlap the clickable area of the white keys, avoiding the need he need for a custom widget.
        addParam(createParam<AriaPianoBlack>(mm2px(Vec(pianoXoffset, pianoYoffset -  5.f)), module, Qqqq::NOTE_PARAM +  1)); // C#
        addParam(createParam<AriaPianoBlack>(mm2px(Vec(pianoXoffset, pianoYoffset - 19.f)), module, Qqqq::NOTE_PARAM +  3)); // D#
        addParam(createParam<AriaPianoBlack>(mm2px(Vec(pianoXoffset, pianoYoffset - 47.f)), module, Qqqq::NOTE_PARAM +  6)); // F#
        addParam(createParam<AriaPianoBlack>(mm2px(Vec(pianoXoffset, pianoYoffset - 61.f)), module, Qqqq::NOTE_PARAM +  8)); // G#
        addParam(createParam<AriaPianoBlack>(mm2px(Vec(pianoXoffset, pianoYoffset - 75.f)), module, Qqqq::NOTE_PARAM + 10)); // A#
    }

    QqqqWidget(Qqqq* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Qqqq.svg")));
        
        drawScrews();
        drawPianoKeys(4.7f, 105.8f, module);


    }
};

Model* modelQqqq = createModel<Qqqq, QqqqWidget>("Qqqq");
Model* modelQuack = createModel<Qqqq, QqqqWidget>("Quack");
Model* modelQ = createModel<Qqqq, QqqqWidget>("Q");
