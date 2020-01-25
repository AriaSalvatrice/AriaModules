#include "plugin.hpp"
#include <ctime>
#include <thread>

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
	
	Test() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);		
	}

	void process(const ProcessArgs& args) override {
		outputs[TEST_OUTPUT + 0].setVoltage( 0.f);
		outputs[TEST_OUTPUT + 1].setVoltage( 0.f);
		outputs[TEST_OUTPUT + 2].setVoltage( 0.f);
		outputs[TEST_OUTPUT + 3].setVoltage( 0.f);
		outputs[TEST_OUTPUT + 4].setVoltage( 0.f);
		outputs[TEST_OUTPUT + 5].setVoltage( 0.f);
		outputs[TEST_OUTPUT + 6].setVoltage( 0.f);
		outputs[TEST_OUTPUT + 7].setVoltage( 0.f);
		outputs[TEST_OUTPUT + 8].setVoltage( 0.f);
		outputs[TEST_OUTPUT + 9].setVoltage( 0.f);
		outputs[TEST_OUTPUT + 10].setVoltage(0.f);
		outputs[TEST_OUTPUT + 11].setVoltage(0.f);
		
		lights[TEST_LIGHT + 0].setBrightness(1.f);
		lights[TEST_LIGHT + 2].setBrightness(1.f);
		lights[TEST_LIGHT + 4].setBrightness(1.f);
		
	}	
};

/*
struct AriaNewJackLight : app::ModuleLightWidget {
	AriaNewJackLight() {
		this->box.size = app::mm2px(math::Vec(8.0, 8.0));
		this->bgColor = nvgRGB(0x0e, 0x69, 0x77);
		this->borderColor = nvgRGB(0x0e, 0x69, 0x77);
	}
	
	void drawLight(const widget::Widget::DrawArgs& args) override {
		float radius = std::min(this->box.size.x, this->box.size.y) / 2.0;
		nvgBeginPath(args.vg);
		nvgCircle(args.vg, radius, radius, radius);

		// Background
		if (this->bgColor.a > 0.0) {
			nvgFillColor(args.vg, this->bgColor);
			nvgFill(args.vg);
		}

		// Foreground
		if (this->color.a > 0.0) {
			nvgFillColor(args.vg, this->color);
			nvgFill(args.vg);
		}

		// Border
		if (this->borderColor.a > 0.0) {
			nvgStrokeWidth(args.vg, app::mm2px(0.2));
			nvgStrokeColor(args.vg, this->borderColor);
			nvgStroke(args.vg);
		}
	}
};

struct AriaNewInputLight : AriaNewJackLight {
	AriaNewInputLight() {
		this->addBaseColor(nvgRGB(0xff, 0xcc, 0x03));
	}
};

struct AriaNewOutputLight : AriaNewJackLight {
	AriaNewOutputLight() {
		this->addBaseColor(nvgRGB(0xfc, 0xae, 0xbb));
	}
};

*/

struct TestWidget : ModuleWidget {
	TestWidget(Test* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Test.svg")));
		
		addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		
		for (int i = 0; i < 12; i++) {
			// addInput(createInput<AriaJackIn>(mm2px(Vec(10.0, 8.0 + i * 10.0)), module, Test::TEST_INPUT + i));
			// addOutput(createOutput<AriaJackOut>(mm2px(Vec(20.0, 8.0 + i * 10.0)), module, Test::TEST_OUTPUT + i));
		}
		

		addChild(createLight<AriaNewInputLight>(mm2px(Vec(30.0, 20.0)), module, Test::TEST_LIGHT + 0));
		addChild(createLight<AriaNewInputLight>(mm2px(Vec(30.0, 30.0)), module, Test::TEST_LIGHT + 1));
		addChild(createLight<AriaNewOutputLight>(mm2px(Vec(30.0, 40.0)), module, Test::TEST_LIGHT + 2));
		addChild(createLight<AriaNewOutputLight>(mm2px(Vec(30.0, 50.0)), module, Test::TEST_LIGHT + 3));
		addChild(createLight<AriaNewInputLight>(mm2px(Vec(30.0, 60.0)), module, Test::TEST_LIGHT + 4));
		addChild(createLight<AriaNewInputLight>(mm2px(Vec(30.0, 70.0)), module, Test::TEST_LIGHT + 5));

		addInput(createInput<AriaJackTransparent>(mm2px(Vec(30.0, 20.0)), module, Test::TEST_INPUT + 0));
		addInput(createInput<AriaJackTransparent>(mm2px(Vec(30.0, 30.0)), module, Test::TEST_INPUT + 1));
		
		addOutput(createOutput<AriaJackTransparent>(mm2px(Vec(30.0, 40.0)), module, Test::TEST_OUTPUT + 2));
		addOutput(createOutput<AriaJackTransparent>(mm2px(Vec(30.0, 50.0)), module, Test::TEST_OUTPUT + 3));

		
	}
};

Model* modelTest = createModel<Test, TestWidget>("Test");