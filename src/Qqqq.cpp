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
    SCALE_MODE
};

// Start high, lower if people complain.
const int REFRESHSCALEDIVIDER = 512;

struct Qqqq : Module {
    enum ParamIds {
        ENUMS(NOTE_PARAM, 12),
        ENUMS(SCALING_PARAM, 4),
        ENUMS(OFFSET_PARAM, 4),
        ENUMS(TRANSPOSE_PARAM, 4),
        ENUMS(TRANSPOSE_MODE_PARAM, 4), 
        ENUMS(SHTH_MODE_PARAM, 4),
        ENUMS(VISUALIZE_PARAM, 4),
        KEY_PARAM,
        SCALE_PARAM,
        SLOT_PARAM,
        PASTE_CLIPBOARD_PARAM,
        KEYBOARD_INPUT_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        ENUMS(CV_INPUT, 4),
        ENUMS(SHTH_INPUT, 4),
        EXT_SCALE_INPUT,
        SLOT_INPUT,
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

    int lcdMode = INIT_MODE;
    float lcdLastInteraction = 0.f;
    float lastKeyKnob = 0.f;
    float lastScaleKnob = 2.f;
    std::array<bool, 12> scale;
    std::array<bool, 12> lastExternalScale;
    Lcd::LcdStatus lcdStatus;
    dsp::ClockDivider refreshScaleDivider;
    
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
        refreshScaleDivider.setDivision(REFRESHSCALEDIVIDER);
        lcdStatus.lcdPage = Lcd::TEXT1_PAGE;
    }

    ~Qqqq(){
    }

    // Update the piano display to match the state of the internal scale
    void scaleToPiano() {
        for (int i = 0; i < 12; i++) params[NOTE_PARAM + i].setValue( (scale[i]) ? 1.f : 0.f );
    }

    // Update the internal scale to match the state of the piano display
    void pianoToScale() {
        
    }


    // The piano buttons are the canonical source of truth.
    // When the External input changes, or the knobs are moved, they override the piano buttons.
    // When an External input is present, the knobs do nothing.
    // These priorities might seem weird, but they're intentional.
    void updateScale() {

        // External scale
        std::array<bool, 12> currentExternalScale;
        if (inputs[EXT_SCALE_INPUT].isConnected()) {
            for (int i = 0; i < 12; i++) currentExternalScale[i] = (inputs[EXT_SCALE_INPUT].getVoltage(i) > 0.f) ? true : false;
            if (currentExternalScale != lastExternalScale) {
                lastExternalScale = currentExternalScale;
                scale = currentExternalScale;
                scaleToPiano();
            }
        }

        // Knobs
        if ( (lastKeyKnob != params[KEY_PARAM].getValue()) || (lastScaleKnob != params[SCALE_PARAM].getValue()) ) {
            if (! inputs[EXT_SCALE_INPUT].isConnected()) {
                scale = Quantizer::validNotesInScaleKey(params[SCALE_PARAM].getValue(), params[KEY_PARAM].getValue());
                scaleToPiano();
            }
        }
        lastKeyKnob = params[KEY_PARAM].getValue();
        lastScaleKnob = params[SCALE_PARAM].getValue();
    }

    void updateExternalOutput() {
        for (int i = 0; i < 12; i++) outputs[EXT_SCALE_OUTPUT].setVoltage( (scale[i]) ? 10.f : 0.f, i);
        outputs[EXT_SCALE_OUTPUT].setChannels(12);
    }

    void process(const ProcessArgs& args) override {

        if (refreshScaleDivider.process()) {
            updateScale();
            updateExternalOutput();
        }

    }
};





///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////





// The LCD knobs
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



// To represent the piano, only 6 distinct key graphics are required: C, E, F, B, White with notch on each side, Black
struct AriaPianoC : SvgSwitchUnshadowed {
    Qqqq* module;
    AriaPianoC(Qqqq* module) {
        this-> module = module;
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/unlit-C.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/yellow-C.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/pink-C.svg")));
        SvgSwitchUnshadowed();
    }
};
struct AriaPianoE : SvgSwitchUnshadowed {
    Qqqq* module;
    AriaPianoE(Qqqq* module) {
        this-> module = module;
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/unlit-E.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/yellow-E.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/pink-E.svg")));
        SvgSwitchUnshadowed();
    }
};
struct AriaPianoF : SvgSwitchUnshadowed {
    Qqqq* module;
    AriaPianoF(Qqqq* module) {
        this-> module = module;
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/unlit-F.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/yellow-F.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/pink-F.svg")));
        SvgSwitchUnshadowed();
    }
};
struct AriaPianoB : SvgSwitchUnshadowed {
    Qqqq* module;
    AriaPianoB(Qqqq* module) {
        this-> module = module;
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/unlit-B.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/yellow-B.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/pink-B.svg")));
        SvgSwitchUnshadowed();
    }
};
struct AriaPianoWhite : SvgSwitchUnshadowed {
    Qqqq* module;
    AriaPianoWhite(Qqqq* module) {
        this-> module = module;
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/unlit-white.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/yellow-white.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/pink-white.svg")));
        SvgSwitchUnshadowed();
    }
};
struct AriaPianoBlack : SvgSwitchUnshadowed {
    Qqqq* module;
    AriaPianoBlack(Qqqq* module) {
        this-> module = module;
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/unlit-black.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/yellow-black.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/piano-buttons/pink-black.svg")));
        SvgSwitchUnshadowed();
    }
};


struct QqqqWidget : ModuleWidget {

    void drawScrews() {
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
    }

    void drawPianoKeys(float xOffset, float yOffset, Qqqq* module) {
        // First we create the white keys only.
        addParam(createModuleParam<AriaPianoC, Qqqq>(    mm2px(Vec(xOffset, yOffset -  0.f)), module, Qqqq::NOTE_PARAM +  0)); // C
        addParam(createModuleParam<AriaPianoWhite, Qqqq>(mm2px(Vec(xOffset, yOffset - 14.f)), module, Qqqq::NOTE_PARAM +  2)); // D
        addParam(createModuleParam<AriaPianoE, Qqqq>(    mm2px(Vec(xOffset, yOffset - 28.f)), module, Qqqq::NOTE_PARAM +  4)); // E
        addParam(createModuleParam<AriaPianoF, Qqqq>(    mm2px(Vec(xOffset, yOffset - 42.f)), module, Qqqq::NOTE_PARAM +  5)); // F
        addParam(createModuleParam<AriaPianoWhite, Qqqq>(mm2px(Vec(xOffset, yOffset - 56.f)), module, Qqqq::NOTE_PARAM +  7)); // G
        addParam(createModuleParam<AriaPianoWhite, Qqqq>(mm2px(Vec(xOffset, yOffset - 70.f)), module, Qqqq::NOTE_PARAM +  9)); // A
        addParam(createModuleParam<AriaPianoB, Qqqq>(    mm2px(Vec(xOffset, yOffset - 84.f)), module, Qqqq::NOTE_PARAM + 11)); // B
        // Then, the black keys, so they overlap the clickable area of the white keys, avoiding the need he need for a custom widget.
        addParam(createModuleParam<AriaPianoBlack, Qqqq>(mm2px(Vec(xOffset, yOffset -  5.f)), module, Qqqq::NOTE_PARAM +  1)); // C#
        addParam(createModuleParam<AriaPianoBlack, Qqqq>(mm2px(Vec(xOffset, yOffset - 19.f)), module, Qqqq::NOTE_PARAM +  3)); // D#
        addParam(createModuleParam<AriaPianoBlack, Qqqq>(mm2px(Vec(xOffset, yOffset - 47.f)), module, Qqqq::NOTE_PARAM +  6)); // F#
        addParam(createModuleParam<AriaPianoBlack, Qqqq>(mm2px(Vec(xOffset, yOffset - 61.f)), module, Qqqq::NOTE_PARAM +  8)); // G#
        addParam(createModuleParam<AriaPianoBlack, Qqqq>(mm2px(Vec(xOffset, yOffset - 75.f)), module, Qqqq::NOTE_PARAM + 10)); // A#
    }

    void drawQuantizerColumn(float xOffset, float yOffset, Qqqq* module, int col) {
        addInput(createInput<AriaJackIn>(mm2px(Vec(xOffset + 0.f, yOffset + 0.f)), module, Qqqq::CV_INPUT + col));
        addParam(createParam<AriaKnob820>(mm2px(Vec(xOffset + 0.f, yOffset + 10.f)), module, Qqqq::SCALING_PARAM + col));
        addParam(createParam<AriaKnob820>(mm2px(Vec(xOffset + 0.f, yOffset + 20.f)), module, Qqqq::OFFSET_PARAM + col));
        addParam(createParam<AriaKnob820>(mm2px(Vec(xOffset + 0.f, yOffset + 30.f)), module, Qqqq::TRANSPOSE_PARAM + col));

        addParam(createParam<AriaPushButton500>(mm2px(Vec(xOffset + 3.5f, yOffset + 40.f)), module, Qqqq::SHTH_MODE_PARAM + col));
        addParam(createParam<AriaPushButton500>(mm2px(Vec(xOffset + -0.5f, yOffset + 42.5f)), module, Qqqq::TRANSPOSE_MODE_PARAM + col));

        addInput(createInput<AriaJackIn>(mm2px(Vec(xOffset + 0.f, yOffset + 50.f)), module, Qqqq::SHTH_INPUT + col));
        addParam(createParam<AriaPushButton820>(mm2px(Vec(xOffset + 0.f, yOffset + 60.f)), module, Qqqq::VISUALIZE_PARAM + col));
        addOutput(createOutput<AriaJackOut>(mm2px(Vec(xOffset + 0.f, yOffset + 70.f)), module, Qqqq::CV_OUTPUT + col));

    }

    QqqqWidget(Qqqq* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Qqqq.svg")));
        
        // Signature
        addChild(createWidget<AriaSignature>(mm2px(Vec(65.f, 114.5f))));

        drawScrews();
        drawPianoKeys(4.7f, 102.8f, module);

        // The LCD will go around here

        // Key, Scale, External
        addParam(createModuleParam<AriaKnob820Scale, Qqqq>(mm2px(Vec(25.f, 29.f)), module, Qqqq::KEY_PARAM));
        addParam(createModuleParam<AriaKnob820Scale, Qqqq>(mm2px(Vec(35.f, 29.f)), module, Qqqq::SCALE_PARAM));
        addInput(createInput<AriaJackIn>(mm2px(Vec(45.f, 29.f)), module, Qqqq::EXT_SCALE_INPUT));
        addOutput(createOutput<AriaJackOut>(mm2px(Vec(55.f, 29.f)), module, Qqqq::EXT_SCALE_OUTPUT));

        // Step programmer will go there
        addParam(createParam<AriaKnob820>(mm2px(Vec(74.f, 53.f)), module, Qqqq::SLOT_PARAM));
        addInput(createInput<AriaJackIn>(mm2px(Vec(84.f, 53.f)), module, Qqqq::SLOT_INPUT));

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
