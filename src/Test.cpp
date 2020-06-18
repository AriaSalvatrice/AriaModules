#include "plugin.hpp"
#include <ctime>
#include <thread>
#include <iomanip>
#include <random>
#include "lib/duktape/duktape.h"

// This module is to make all sorts of tests without having to recompile too much or deal with complex code interactions.

struct Test : Module {
    enum ParamIds {
        ENUMS(TEST_PARAM, 12),
        NUM_PARAMS
    };
    enum InputIds {
        ENUMS(TEST_INPUT, 12),
        NUM_INPUTS
    };
    enum OutputIds {
        ENUMS(TEST_OUTPUT, 12),
        NUM_OUTPUTS
    };
    enum LightIds {
        ENUMS(TEST_LIGHT, 12),
        NUM_LIGHTS
    };
    dsp::ClockDivider testDivider;
    duk_context* ctx = NULL;
    
    Test() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        testDivider.setDivision(44);
        ctx = duk_create_heap_default();
        DEBUG("HI");
    }

    ~Test(){
        if (ctx) duk_destroy_heap(ctx);
    }

    void process(const ProcessArgs& args) override {

    }
};


struct TestWidget : ModuleWidget {
    TestWidget(Test* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Test.svg")));
        
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
                
        for (int i = 0; i < 12; i++) {
            addInput(createInput<AriaJackIn>(mm2px(Vec(10.0, 8.0 + i * 10.0)), module, Test::TEST_INPUT + i));
            addOutput(createOutput<AriaJackOut>(mm2px(Vec(20.0, 8.0 + i * 10.0)), module, Test::TEST_OUTPUT + i));
        }

    }
};

Model* modelTest = createModel<Test, TestWidget>("Test");
