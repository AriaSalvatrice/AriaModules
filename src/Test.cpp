#include "plugin.hpp"
#include <ctime>
#include <thread>
#include <iomanip>
#include <random>

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

    
    Test() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        testDivider.setDivision(44);

    }

    void process(const ProcessArgs& args) override {
        // outputs[TEST_OUTPUT + 0].setVoltage( 0.f);
        // outputs[TEST_OUTPUT + 1].setVoltage( 0.f);
        // outputs[TEST_OUTPUT + 2].setVoltage( 0.f);
        // outputs[TEST_OUTPUT + 3].setVoltage( 0.f);
        // outputs[TEST_OUTPUT + 4].setVoltage( 0.f);
        // outputs[TEST_OUTPUT + 5].setVoltage( 0.f);
        // outputs[TEST_OUTPUT + 6].setVoltage( 0.f);
        // outputs[TEST_OUTPUT + 7].setVoltage( 0.f);
        // outputs[TEST_OUTPUT + 8].setVoltage( 0.f);
        // outputs[TEST_OUTPUT + 9].setVoltage( 0.f);
        // outputs[TEST_OUTPUT + 10].setVoltage(0.f);
        // outputs[TEST_OUTPUT + 11].setVoltage(0.f);

        // lights[TEST_LIGHT + 0].setBrightness(1.f);

        // float random = 0;
        
        if (testDivider.process()) {
            std::mt19937 generator(7777);	
            std::bernoulli_distribution coinFlip(0.5);
            outputs[TEST_OUTPUT + 0].setVoltage( (coinFlip(generator)) ? 0.f : 10.f );
            outputs[TEST_OUTPUT + 1].setVoltage( (coinFlip(generator)) ? 0.f : 10.f );
            outputs[TEST_OUTPUT + 2].setVoltage( (coinFlip(generator)) ? 0.f : 10.f );
            outputs[TEST_OUTPUT + 3].setVoltage( (coinFlip(generator)) ? 0.f : 10.f );
            outputs[TEST_OUTPUT + 4].setVoltage( (coinFlip(generator)) ? 0.f : 10.f );
            outputs[TEST_OUTPUT + 5].setVoltage( (coinFlip(generator)) ? 0.f : 10.f );
            outputs[TEST_OUTPUT + 6].setVoltage( (coinFlip(generator)) ? 0.f : 10.f );
            outputs[TEST_OUTPUT + 7].setVoltage( (coinFlip(generator)) ? 0.f : 10.f );
            outputs[TEST_OUTPUT + 8].setVoltage( (coinFlip(generator)) ? 0.f : 10.f );
        }
        
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
