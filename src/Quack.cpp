#include "plugin.hpp"
#include "javascript.hpp"
#include "javascript-libraries.hpp"

/*
Names: Quack, Quick, Quark?, Quasar?, Quokka?, Quorum?, Q<

UI:
- A piano on the left
- Lots of slots. Let's say 64?
- A duck
- External I/O
- LCD
*/

struct Quack : Module {
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
    
    Quack() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    }

    ~Quack(){
    }

    void updateScale(const ProcessArgs& args) {
        for (int i = 0; i < 12; i++) scale[i] = (params[NOTE_PARAM + i].getValue() == 1.f) ? true : false;
        
    }

    void process(const ProcessArgs& args) override {
        updateScale(args);
    }
};


struct QuackWidget : ModuleWidget {
    QuackWidget(Quack* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Quack.svg")));
        
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam(createParam<AriaPushButton820>(mm2px(Vec(10.0, 115.0)), module, Quack::NOTE_PARAM +  0)); // C
        addParam(createParam<AriaPushButton820>(mm2px(Vec( 4.0, 105.0)), module, Quack::NOTE_PARAM +  1)); // C#
        addParam(createParam<AriaPushButton820>(mm2px(Vec(10.0,  95.0)), module, Quack::NOTE_PARAM +  2)); // D
        addParam(createParam<AriaPushButton820>(mm2px(Vec( 4.0,  85.0)), module, Quack::NOTE_PARAM +  3)); // D#
        addParam(createParam<AriaPushButton820>(mm2px(Vec(10.0,  75.0)), module, Quack::NOTE_PARAM +  4)); // E
        addParam(createParam<AriaPushButton820>(mm2px(Vec(10.0,  65.0)), module, Quack::NOTE_PARAM +  5)); // F
        addParam(createParam<AriaPushButton820>(mm2px(Vec( 4.0,  55.0)), module, Quack::NOTE_PARAM +  6)); // F#
        addParam(createParam<AriaPushButton820>(mm2px(Vec(10.0,  45.0)), module, Quack::NOTE_PARAM +  7)); // G
        addParam(createParam<AriaPushButton820>(mm2px(Vec( 4.0,  35.0)), module, Quack::NOTE_PARAM +  8)); // G#
        addParam(createParam<AriaPushButton820>(mm2px(Vec(10.0,  25.0)), module, Quack::NOTE_PARAM +  9)); // A
        addParam(createParam<AriaPushButton820>(mm2px(Vec( 4.0,  15.0)), module, Quack::NOTE_PARAM + 10)); // A#
        addParam(createParam<AriaPushButton820>(mm2px(Vec(10.0,  05.0)), module, Quack::NOTE_PARAM + 11)); // B

    }
};

Model* modelQuack = createModel<Quack, QuackWidget>("Quack");
