#include "plugin.hpp"
// This will contain both Bendlet and Big Bend.

/* TODO
Stupid idiot check
[X] Verify that pitchbend is indeed a -5~+5v signal centered on 0
    > Yup, but it doesn't go all the way up to 5. (4.99939)

Widget
[X] Make the widget work at all
[X] Make the widget output something on debug
[ ] Override the widget's position, spring back center
[ ] Override only if user isn't moving it with MIDI (not CV)
[ ] Override only if user isn't dragging it
[ ] Add smoothing to springing back center

Processing
[X] Make input go to output without processing
[ ] Make it polyphonic from the start
[-] Make input processed by widget (fixed range)
[ ] Make range up and down configurable (right click)
[ ] Make output quantized to nearest 12TET semitone
[ ] Generate scales (root note + rules > 12 bit)
[ ] Make output quantized to scale (even when bending)
[ ] Quantize input and make PB range in scale degrees

Big Bend
[ ] Draw Big Ben, bent, the building a piano, the clock a circle of 5ths
[ ] Design the UI on paper
[ ] Design the UI for real
[ ] Figure out how to have 2 modules in a single .cpp file
[ ] Provide all the right click options as UI options
[ ] Provide CV input for every option that makes sense
*/



struct Bendlet : Module {
	enum ParamIds {
		PB_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		PITCH_INPUT,
		PB_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		BENT_OUTPUT,
		DEBUG_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Bendlet() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		// TODO: can I dynamically alter display in semitones?
		configParam(PB_PARAM, -1.0, 1.0, 0.0, "Pitchbend", " semitones", 0.0, 12.0); 
	}

	void process(const ProcessArgs& args) override {
		float pb = params[PB_PARAM].getValue();
		outputs[DEBUG_OUTPUT].setVoltage(pb);
		
		// Bending up and down an octave for now - makes the math simple.
		float bent = inputs[PITCH_INPUT].getVoltage();
		bent += pb;
		outputs[BENT_OUTPUT].setVoltage(bent);
	}
};

struct AriaPbSlider : SvgSlider {
	AriaPbSlider() {
		setBackgroundSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pb-bg.svg")));
		setHandleSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pb-knob.svg")));
		maxHandlePos = mm2px(Vec(1.9, 2.2));
		minHandlePos = mm2px(Vec(1.9, 43.0));
	}
};


struct BendletWidget : ModuleWidget {
	BendletWidget(Bendlet* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Bendlet.svg")));

		// Signature
		addChild(createWidget<AriaSignature>(mm2px(Vec(1.0, 114.538))));

		// Screws
		addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// Input
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 79.0)), module, Bendlet::PITCH_INPUT));
		addInput(createInputCentered<AriaJackIn>(mm2px(Vec(7.62, 92.0)), module, Bendlet::PB_INPUT));

		// Output
		addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(7.62, 105.0)), module, Bendlet::BENT_OUTPUT));
		addOutput(createOutputCentered<AriaJackOut>(mm2px(Vec(7.62, 117.0)), module, Bendlet::DEBUG_OUTPUT));

		// PB Wheel
		addParam(createParam<AriaPbSlider>(mm2px(Vec(2.12, 18.0)), module, Bendlet::PB_PARAM));
	}
};


Model* modelBendlet = createModel<Bendlet, BendletWidget>("Bendlet");