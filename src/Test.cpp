#include "plugin.hpp"
#include <ctime>
#include <thread>
#include <iomanip>

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
	
	std::string text;
	
	bool dirty;
	
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
		
		text = "woof";
		
	}	
};

// Possibly useful
// https://github.com/squinkylabs/SquinkyVCV/blob/master/sqsrc/util/DrawTimer.h


// Originally From GTG
// Pixel fonts are zoom dependent. This method is not usable.
// 

// Framebuffer should go around it.
// https://community.vcvrack.com/t/framebufferwidget-question/3041


/*

// we want to draw stuff procedurally only when things change
// suppose we have SomeModule with an integer "someValue" and a bool "dirty" 
// whenever "someValue" changes "dirty" gets set to true

// create a container for widgets that draw things
struct SomeWidgetBuffer : FramebufferWidget{
  SomeModule *module;
  SomeWidgetBuffer(SomeModule *m){
    module = m;
  }
  void step() override{
    if(module->dirty){
      FramebufferWidget::dirty = true;
      module->dirty = false;
    }
    FramebufferWidget::step();
  }
};

// create widget that draws something based on values of the module
struct SomeDrawingWidget : VirtualWidget{
  SomeModule *module;
  SomeDrawingWidget(SomeModule *m){
    module = m;
    box.pos = Vec(0,0);
    box.size = Vec(100, 100);
  }
  void draw(NVGcontext *vg) override{
    printf("drawing stuff only once\n");
    nvgBeginPath(vg);
    nvgRect(vg, 0, 0,box.size.x, box.size.y);
    nvgFillColor(vg, nvgRGB(module->someValue,0,0));
    nvgFill(vg);
  }
};

// then somewhere inside the ModuleWidget's constructor
{
  SomeWidgetBuffer *fb = new SomeWidgetBuffer(module);
  SomeDrawingWidget *dw = new SomeDrawingWidget(module);
  fb->addChild(dw);
  addChild(fb);
}
*/


/*
struct LCDFramebuffer : FramebufferWidget{
  Test *module;
  LCDFramebuffer(Test *m){
    module = m;
  }
  void step() override{
    if(module->dirty){
      FramebufferWidget::dirty = true;
      module->dirty = false;
    }
    FramebufferWidget::step();
  }
};

struct LCDWidget : TransparentWidget {
	Test *module;
	std::array<std::shared_ptr<Svg>, 95> asciiSvg; // 32 to 126, the printable range
	std::shared_ptr<Svg> pianoTestSvg;
	int testImage;

	LCDWidget() {
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

*/



struct LCDWidget : FramebufferWidget {
	struct LCDDrawWidget : TransparentWidget {
		Test *module;
		std::array<std::shared_ptr<Svg>, 95> asciiSvg; // 32 to 126, the printable range
		std::shared_ptr<Svg> pianoTestSvg;
		int testImage;
	
		LCDDrawWidget() {
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
			// std::string text = module ? module->text : "";
			std::string text = "Hello";
			text.append(11, ' '); // Ensure the string is long enough
			for (int i = 0; i < 11; i++) {
				char c = text.at(i);
				svgDraw(args.vg, asciiSvg[ c - 32 ]->handle);
				nvgTranslate(args.vg, 6, 0);
			}
			nvgRestore(args.vg);
		}
	};
	
	Test* module;
	LCDDrawWidget* w;
	
	LCDWidget() {
		w = new LCDDrawWidget();
		addChild(w);
	}
	
	void step() override {

		FramebufferWidget::dirty = true;
		FramebufferWidget::step();

	}
	
	/*
	
	MODULE* module;
	MazeGridDrawWidget* w;
	
	MazeGridWidget(MODULE* module) {
		this->module = module;
		w = new MazeGridDrawWidget(module);
		addChild(w);
	}

	void step() override{
		if (module && module->gridDirty) {
			FramebufferWidget::dirty = true;
			w->box.size = box.size;
			w->gridColor = module->currentState == MODULESTATE::EDIT ? color::mult(color::WHITE, 0.35f) : color::WHITE;
			module->gridDirty = false;
		}
		FramebufferWidget::step();
	}
	
	
	*/
	
	
	
};


struct TestWidget : ModuleWidget {
	TestWidget(Test* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Test.svg")));
		
		addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		
		
		LCDWidget *lcd= createWidget<LCDWidget>(mm2px(Vec(13.7, 46.7))); // + 1.5,0.4 from margin
		lcd->module = module;
		addChild(lcd);
		
		
		
		for (int i = 0; i < 12; i++) {
			// addInput(createInput<AriaJackIn>(mm2px(Vec(10.0, 8.0 + i * 10.0)), module, Test::TEST_INPUT + i));
			// addOutput(createOutput<AriaJackOut>(mm2px(Vec(20.0, 8.0 + i * 10.0)), module, Test::TEST_OUTPUT + i));
		}

	}
};

Model* modelTest = createModel<Test, TestWidget>("Test");