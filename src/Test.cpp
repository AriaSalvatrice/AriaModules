#include "plugin.hpp"
#include <ctime>
#include <thread>
#include <iomanip>
#include <random>
#include "portablesequence.hpp"

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

        PortableSequence::Sequence sequence;
        PortableSequence::Note note1;
        note1.start = 12.f;
        note1.pitch = 1.15f;
        note1.length = 4.f;
        note1.velocity = 1.5f;
        PortableSequence::Note note2;
        note2.start = 7.f;
        note2.pitch = 9454.15f;
        note2.length = 0.05f;
        PortableSequence::Note note3;
        note3.start = 8.f;
        note3.pitch = -4.25f;
        note3.length = 2.0f;
        note3.playProbability = 0.6f;
        sequence.addNote(note1);
        sequence.addNote(note2);
        sequence.addNote(note3);
        sequence.clampValues();
        sequence.calculateLength();
        sequence.sort();
        sequence.toClipboard();
    }

    ~Test(){
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
