#include "plugin.hpp"
#include "javascript.hpp"
#include "javascript-libraries.hpp"

/*
Names: Quatherine's Quality Quad Quantizer, Quack, Q<

UI:
- Lots of slots. Let's say 64?
- Duck
- External I/O
- LCD
- Import button

*/

/* Quatherine's Quality Quad Quantizer, Quack, Q<
   All three modules are the same, with different widgets.

*/ 

struct Qqqq : Module {
    enum ParamIds {
        ENUMS(NOTE_PARAM, 12),
        NUM_PARAMS
    };
    enum InputIds {
        NUM_INPUTS
    };
    enum OutputIds {
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    std::array<bool, 12> scale;
    std::array<bool, 12> litPianoKeys;
    dsp::ClockDivider testDivider;
    
    Qqqq() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(NOTE_PARAM +  0, 0.f, 1.f, 0.f, "C");
        configParam(NOTE_PARAM +  1, 0.f, 1.f, 0.f, "C#");
        configParam(NOTE_PARAM +  2, 0.f, 1.f, 0.f, "D");
        configParam(NOTE_PARAM +  3, 0.f, 1.f, 0.f, "D#");
        configParam(NOTE_PARAM +  4, 0.f, 1.f, 0.f, "E");
        configParam(NOTE_PARAM +  5, 0.f, 1.f, 0.f, "F");
        configParam(NOTE_PARAM +  6, 0.f, 1.f, 0.f, "F#");
        configParam(NOTE_PARAM +  7, 0.f, 1.f, 0.f, "G");
        configParam(NOTE_PARAM +  8, 0.f, 1.f, 0.f, "G#");
        configParam(NOTE_PARAM +  9, 0.f, 1.f, 0.f, "A");
        configParam(NOTE_PARAM + 10, 0.f, 1.f, 0.f, "A#");
        configParam(NOTE_PARAM + 11, 0.f, 1.f, 0.f, "B");
        testDivider.setDivision(88888);
        litPianoKeys[0] = false;
        litPianoKeys[1] = true;
        litPianoKeys[2] = false;
        litPianoKeys[3] = false;
        litPianoKeys[4] = false;
        litPianoKeys[5] = true;
        litPianoKeys[6] = false;
        litPianoKeys[7] = false;
        litPianoKeys[8] = true;
        litPianoKeys[9] = false;
        litPianoKeys[10] = false;
        litPianoKeys[11] = false;
    }

    ~Qqqq(){
    }

    void updateScale(const ProcessArgs& args) {
        for (int i = 0; i < 12; i++) scale[i] = (params[NOTE_PARAM + i].getValue() == 1.f) ? true : false;
        
    }

    void process(const ProcessArgs& args) override {
        updateScale(args);
    }
};





///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////





// Piano keys have three states, but users can only manually toggle two.
// The third is read from the module.
struct PianoKey : SvgSwitchUnshadowed {
    bool isLit;
    bool lightUp;

    PianoKey() {
        SvgSwitchUnshadowed();
    }

    void draw(const DrawArgs &args) override {
	// if (!frames.empty() && paramQuantity) {
	// 	int index = (int) std::round(paramQuantity->getValue() - paramQuantity->getMinValue());
	// 	index = math::clamp(index, 0, (int) frames.size() - 1);
	// 	sw->setSvg(frames[index]);
	// 	fb->dirty = true;
	// }
        sw->setSvg(frames[2]);
        fb->dirty = true;
        SvgSwitchUnshadowed::draw(args);
    }

};

template <class TParamWidget>
TParamWidget* createPianoParam(math::Vec pos, engine::Module* module,  Qqqq* qqqqInstance, int keyNumber, int paramId) {
	TParamWidget* o = new TParamWidget;
	o->box.pos = pos;
	if (module) {
		o->paramQuantity = module->paramQuantities[paramId];
        o->lightUp = qqqqInstance->litPianoKeys[keyNumber];
	}
	return o;
}

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
    QqqqWidget(Qqqq* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Qqqq.svg")));
        
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Piano keys
        float pianoXoffset = 4.7f;
        float pianoYoffset = 105.8f;
        
        addParam(createPianoParam<AriaPianoC>(    mm2px(Vec(pianoXoffset, pianoYoffset -  0.f)), module, module,  0, Qqqq::NOTE_PARAM +  0)); // C
        addParam(createPianoParam<AriaPianoWhite>(mm2px(Vec(pianoXoffset, pianoYoffset - 14.f)), module, module,  2, Qqqq::NOTE_PARAM +  2)); // D
        addParam(createPianoParam<AriaPianoE>(    mm2px(Vec(pianoXoffset, pianoYoffset - 28.f)), module, module,  4, Qqqq::NOTE_PARAM +  4)); // E
        addParam(createPianoParam<AriaPianoF>(    mm2px(Vec(pianoXoffset, pianoYoffset - 42.f)), module, module,  5, Qqqq::NOTE_PARAM +  5)); // F
        addParam(createPianoParam<AriaPianoWhite>(mm2px(Vec(pianoXoffset, pianoYoffset - 56.f)), module, module,  7, Qqqq::NOTE_PARAM +  7)); // G
        addParam(createPianoParam<AriaPianoWhite>(mm2px(Vec(pianoXoffset, pianoYoffset - 70.f)), module, module,  9, Qqqq::NOTE_PARAM +  9)); // A
        addParam(createPianoParam<AriaPianoB>(    mm2px(Vec(pianoXoffset, pianoYoffset - 84.f)), module, module, 11, Qqqq::NOTE_PARAM + 11)); // B
        // Then, the black keys, so they overlap the clickable area of the white keys, avoiding the need he need for a custom widget.
        addParam(createPianoParam<AriaPianoBlack>(mm2px(Vec(pianoXoffset, pianoYoffset -  5.f)), module, module,  1, Qqqq::NOTE_PARAM +  1)); // C#
        addParam(createPianoParam<AriaPianoBlack>(mm2px(Vec(pianoXoffset, pianoYoffset - 19.f)), module, module,  3, Qqqq::NOTE_PARAM +  3)); // D#
        addParam(createPianoParam<AriaPianoBlack>(mm2px(Vec(pianoXoffset, pianoYoffset - 47.f)), module, module,  6, Qqqq::NOTE_PARAM +  6)); // F#
        addParam(createPianoParam<AriaPianoBlack>(mm2px(Vec(pianoXoffset, pianoYoffset - 61.f)), module, module,  8, Qqqq::NOTE_PARAM +  8)); // G#
        addParam(createPianoParam<AriaPianoBlack>(mm2px(Vec(pianoXoffset, pianoYoffset - 75.f)), module, module, 10, Qqqq::NOTE_PARAM + 10)); // A#

    }
};

Model* modelQqqq = createModel<Qqqq, QqqqWidget>("Qqqq");
Model* modelQuack = createModel<Qqqq, QqqqWidget>("Quack");
Model* modelQ = createModel<Qqqq, QqqqWidget>("Q");
