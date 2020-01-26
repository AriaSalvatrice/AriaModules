#include "plugin.hpp"
#include <ctime>
#include <thread>
#include <iomanip>

// This module is to make all sorts of tests without having to recompile too much or deal with complex code interactions.

// Possibly useful
// https://github.com/squinkylabs/SquinkyVCV/blob/master/sqsrc/util/DrawTimer.h

// https://community.vcvrack.com/t/framebufferwidget-question/3041
// https://github.com/stoermelder/vcvrack-packone/blob/f8b93893ff54cb96941aa6126a5a20f9bc92fdaa/src/Maze.cpp



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
	
	std::string text;
	bool dirty;
	bool scale[12];
	dsp::ClockDivider lcdDivider; 
	
	Test() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		lcdDivider.setDivision(100000);
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

		scale[0] = true;
		scale[1] = false;
		scale[2] = true;
		scale[3] = false;
		scale[4] = true;
		scale[5] = false;
		scale[6] = true;
		scale[7] = false;
		scale[8] = true;
		scale[9] = true;
		scale[10] = false;
		scale[11] = true;

		if (lcdDivider.process()) {
				lcdDivider.setDivision(args.sampleRate * 2); // Any better way to set it up from the constructor?
				text = (text == "woof") ? "w-woof!!" : "woof";
				dirty = true;
		}
	}	
};



struct LCDWidget : FramebufferWidget {
	struct LCDDrawWidget : TransparentWidget {
		Test *module;
		std::array<std::shared_ptr<Svg>, 95> asciiSvg; // 32 to 126, the printable range
		std::array<std::shared_ptr<Svg>, 24> pianoSvg; // 0..11: off, 12..23 = on
		std::shared_ptr<Svg> pianoTestSvg;
		int testImage;
	
		LCDDrawWidget(Test *module) {
			this->module = module;
			box.size = mm2px(Vec(36.0, 10.0));
			pianoTestSvg = APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/lcd/piano/pianotest2.svg"));
			for (int i = 0; i < 95; i++) {
				asciiSvg[i] = APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/lcd/Fixed_v01/" + std::to_string(i + 32) + ".svg"));
			}
		}
	
		void draw(const DrawArgs &args) override {	
			nvgScale(args.vg, 1.5, 1.5);
			nvgSave(args.vg);
			// Piano
			svgDraw(args.vg, pianoTestSvg->handle);
			nvgTranslate(args.vg, 0, 11);
			// 11 character display
			std::string text = module ? module->text : "";
			text.append(11, ' '); // Ensure the string is long enough
			for (int i = 0; i < 11; i++) {
				char c = text.at(i);
				svgDraw(args.vg, asciiSvg[ c - 32 ]->handle);
				nvgTranslate(args.vg, 6, 0);
			}
			nvgRestore(args.vg);
		}
	};
	
	Test *module;
	LCDDrawWidget *drawWidget;
	
	LCDWidget(Test *module) {
		this->module = module;
		drawWidget = new LCDDrawWidget(module);
		addChild(drawWidget);
	}
	
	void step() override {
		if (module && module->dirty) {
			FramebufferWidget::dirty = true;
			module->dirty = false;
		}
		FramebufferWidget::step();
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
		
		//LCDWidget *lcd= createWidget<LCDWidget>(mm2px(Vec(13.7, 46.7))); // + 1.5,0.4 from margin
		//lcd->module = module;
		//addChild(lcd);
		
		LCDWidget *lcd = new LCDWidget(module);
		lcd->box.pos = mm2px(Vec(13.7, 46.7));
		addChild(lcd);
		
		for (int i = 0; i < 12; i++) {
			// addInput(createInput<AriaJackIn>(mm2px(Vec(10.0, 8.0 + i * 10.0)), module, Test::TEST_INPUT + i));
			// addOutput(createOutput<AriaJackOut>(mm2px(Vec(20.0, 8.0 + i * 10.0)), module, Test::TEST_OUTPUT + i));
		}

	}
};

Model* modelTest = createModel<Test, TestWidget>("Test");