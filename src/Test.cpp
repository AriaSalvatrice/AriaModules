#include "plugin.hpp"
#include <ctime>
#include <thread>

// This module is to make all sorts of tests without having to recompile 
// too much when I want to do things that would go in header files.

struct Test : Module {
	enum ParamIds {
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
		NUM_LIGHTS
	};
	
	int bpm = 30;
	int phaseCounter = 0;
	float phase = 0.f;
	dsp::PulseGenerator pulseThirtySecondGenerator, pulseSixteenthGenerator, pulseEighthGenerator, pluseQuarterGenerator;
	bool pulseThirtySecond, pulseSixteenth, pulseEighth, pluseQuarter = false;
	int thirtySecondCounter, sixteenthCounter, eighthCounter, quarterCounter, quarterInBarCounter, barCounter;
	
	Test() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);		
	}
	
	// I have no idea whatsoever how a clock is supposed to be implemented btw.
	void updateClock(const ProcessArgs& args) {
		phase += bpm / 60.f * 512.f / args.sampleRate; // High resolution to get a smooth ramp
		if (phase >= 1.0) {
			phase -= 1.0;
			// phaseCounter = ( phaseCounter == 512 ) ? 0 : phaseCounter + 1;
			if (phaseCounter > 0 ) {
				if ( phaseCounter % 512 == 0 ) {
					pluseQuarterGenerator.trigger(1e-3f); 
					quarterCounter = ( quarterCounter == 15 ? 0 : quarterCounter + 1 );
					if ( quarterInBarCounter == 3 ) {
						quarterInBarCounter = 0;
						barCounter = ( barCounter == 15 ? 0 : barCounter + 1 );
					} else {
						quarterInBarCounter++;
					}
				}
				if ( phaseCounter % 256 == 0 ) {
					pulseEighthGenerator.trigger(1e-3f); 
					eighthCounter = ( eighthCounter == 15 ? 0 : eighthCounter + 1 );
				}
				if ( phaseCounter % 128 == 0 ) {
					pulseSixteenthGenerator.trigger(1e-3f); 
					sixteenthCounter = ( sixteenthCounter == 15 ? 0 : sixteenthCounter + 1 );
				}
				if ( phaseCounter % 64 == 0 ) {
					pulseThirtySecondGenerator.trigger(1e-3f);
					thirtySecondCounter = ( thirtySecondCounter == 15 ? 0 : thirtySecondCounter + 1 );
				}
			}
			
			phaseCounter = ( phaseCounter == 512 ) ? 0 : phaseCounter + 1;
			
		}
		pluseQuarter = pluseQuarterGenerator.process(args.sampleTime);
		pulseEighth = pulseEighthGenerator.process(args.sampleTime);
		pulseSixteenth = pulseSixteenthGenerator.process(args.sampleTime);
		pulseThirtySecond = pulseThirtySecondGenerator.process(args.sampleTime);
	}
	
	void process(const ProcessArgs& args) override {
		updateClock(args);
		outputs[TEST_OUTPUT + 0].setVoltage( pulseThirtySecond ? 10.f : 0.f ); // 32nd pulse
		outputs[TEST_OUTPUT + 1].setVoltage( pulseSixteenth ? 10.f : 0.f ); // 16th pulse
		outputs[TEST_OUTPUT + 2].setVoltage( pulseEighth ? 10.f : 0.f ); // 8th pulse
		outputs[TEST_OUTPUT + 3].setVoltage( pluseQuarter ? 10.f : 0.f ); // 4th pulse
		outputs[TEST_OUTPUT + 4].setVoltage( ((phase + phaseCounter) / 512.f * 2.5f) + quarterInBarCounter * 2.5f ); // bar ramp
		outputs[TEST_OUTPUT + 5].setVoltage( (phase + phaseCounter) / 512.f * 10.f );  // 4th note ramp
		outputs[TEST_OUTPUT + 6].setVoltage( (phase + phaseCounter % 256) / 256.f * 10.f );  // 8th ramp?
		outputs[TEST_OUTPUT + 7].setVoltage( (phase + phaseCounter % 128) / 128.f * 10.f );  // 16th ramp?????
		outputs[TEST_OUTPUT + 8].setVoltage( (phase + phaseCounter % 64) / 64.f * 10.f );  // 32nd ramp!!!!!!!!!!
		outputs[TEST_OUTPUT + 9].setVoltage( thirtySecondCounter / 10.f );  // yeah
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