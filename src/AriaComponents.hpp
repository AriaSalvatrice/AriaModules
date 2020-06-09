#pragma once

using namespace rack;
extern Plugin* pluginInstance;

// - TODO: Remove every single dependency on the component library, since it is not open-source. 

//////////////////////////////// Decorative

// These require a standard <3-shaped screwdriver, provided complimentary with every purchasee. 
struct AriaScrew : SvgScrew {
	AriaScrew() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/screw.svg")));
	}
};

// My personal brand, featuring the Cool S.
struct AriaSignature : SvgWidget {
	AriaSignature() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/signature.svg")));
	}
};



//////////////////////////////// Jacks

// Input jacks are always lit yellow.
struct AriaJackIn : SVGPort {
	AriaJackIn() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/jack-in.svg")));
	}
};

// This output jack is always lit pink.
struct AriaJackOut : SVGPort {
	AriaJackOut() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/jack-out.svg")));
	}
};

// This output jack has a transparent ring, to display a light behind it. 
struct AriaJackTransparent : SVGPort {
	AriaJackTransparent() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/jack-transparent.svg")));
	}
};

/////////////// Old style lights - TODO: Remove dependency on component library

// Still used for a few things! 
template <typename TBase = GrayModuleLightWidget>
struct TOutputLight : TBase {
	TOutputLight() {
		this->addBaseColor(nvgRGB(0xfc, 0xae, 0xbb));
	}
};
typedef TOutputLight<> OutputLight;

template <typename TBase = GrayModuleLightWidget>
struct TInputLight : TBase {
	TInputLight() {
		this->addBaseColor(nvgRGB(0xff, 0xcc, 0x03));
	}
};
typedef TInputLight<> InputLight;


/////////////// Jack lights

// Those lights should be added before transparent jacks, at the same position.
struct AriaJackLight : app::ModuleLightWidget {
	AriaJackLight() {
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

struct AriaInputLight : AriaJackLight {
	AriaInputLight() {
		this->addBaseColor(nvgRGB(0xff, 0xcc, 0x03));
	}
};

struct AriaOutputLight : AriaJackLight {
	AriaOutputLight() {
		this->addBaseColor(nvgRGB(0xfc, 0xae, 0xbb));
	}
};




//////////////////////////////// Switches

// 5.00mm switch. Yellow when lit.
struct AriaPushButton500 : SvgSwitch {
	AriaPushButton500() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-500-off.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-500-on.svg")));
	}
};

struct AriaPushButton500Momentary : SvgSwitch {
	AriaPushButton500Momentary() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-500-off.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-500-on.svg")));
		momentary = true;
	}
};

// 7.00mm switch. Samesies.
struct AriaPushButton700 : SvgSwitch {
	AriaPushButton700() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-700-off.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-700-on.svg")));
	}
};

struct AriaPushButton700Momentary : SvgSwitch {
	AriaPushButton700Momentary() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-700-off.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-700-on.svg")));
		momentary = true;
	}
};

// 8.20mm switch. Yes.
struct AriaPushButton820 : SvgSwitch {
	AriaPushButton820() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-off.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-on.svg")));
	}
};

struct AriaPushButton820Momentary : SvgSwitch {
	AriaPushButton820Momentary() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-off.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-on.svg")));
		momentary = true;
	}
};

// Rocker siwtch, horizontal. Left is default
struct AriaRockerSwitchHorizontal800 : SvgSwitch {
	AriaRockerSwitchHorizontal800() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/rocker-switch-800-l.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/rocker-switch-800-r.svg")));
	}
};

// Rocker siwtch, horizontal. I/O (On/Off). Left is default (Off).
struct AriaRockerSwitchHorizontalIO800 : SvgSwitch {
	AriaRockerSwitchHorizontalIO800() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/rocker-switch-io-800-l.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/rocker-switch-io-800-r.svg")));
	}
};

struct AriaRockerSwitchVertical800 : SvgSwitch {
	AriaRockerSwitchVertical800() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/rocker-switch-800-u.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/rocker-switch-800-d.svg")));
	}
};


//////////////////////////////// Knobs

struct AriaKnob820 : app::SvgKnob {
	AriaKnob820() {
		minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/knob-820.svg")));
	}
};


struct AriaKnob820Transparent : app::SvgKnob {
	AriaKnob820Transparent() {
		minAngle = -0.83 * M_PI;
		maxAngle = 0.83 * M_PI;
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/knob-820-transparent.svg")));
	}
};



//////////////////////////////// LCD

// The framebuffer holding the Draw widget.
template <typename T>
struct LCDFramebufferWidget : FramebufferWidget{
	T *module;
	LCDFramebufferWidget(T *m){
		module = m;
	}

	void step() override{
		if (module) { // Required to avoid crashing module browser
			if(module->lcdDirty){
				FramebufferWidget::dirty = true;
				module->lcdDirty = false;
			}
			FramebufferWidget::step();
		}
	}
};



template <typename T>
struct LCDDrawWidgetPiano : TransparentWidget {
	T *module;
	std::array<std::shared_ptr<Svg>, 95> asciiSvg; // 32 to 126, the printable range
	std::array<std::shared_ptr<Svg>, 24> pianoSvg; // 0..11: Unlit, 12..23 = Lit
	int testImage;

	LCDDrawWidgetPiano(T *module) {
		this->module = module;
		if (module) {
			box.size = mm2px(Vec(36.0, 10.0));
			for (int i = 0; i < 12; i++) // Unlit
				pianoSvg[i] = APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/lcd/piano/u" + std::to_string(i) + ".svg"));
			for (int i = 0; i < 12; i++) // Lit
				pianoSvg[i + 12] = APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/lcd/piano/l" + std::to_string(i) + ".svg"));
			for (int i = 0; i < 95; i++)
				asciiSvg[i] = APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/lcd/Fixed_v01/" + std::to_string(i + 32) + ".svg"));
		}
	}

	void draw(const DrawArgs &args) override {
		if (module) {
			nvgScale(args.vg, 1.5, 1.5);
			nvgSave(args.vg);
			
			// Piano
			svgDraw(args.vg, pianoSvg[(module->scale[0])  ? 12 :  0 ]->handle);
			nvgTranslate(args.vg, 6, 0);
			svgDraw(args.vg, pianoSvg[(module->scale[1])  ? 13 :  1 ]->handle);
			nvgTranslate(args.vg, 5, 0);
			svgDraw(args.vg, pianoSvg[(module->scale[2])  ? 14 :  2 ]->handle);
			nvgTranslate(args.vg, 5, 0);
			svgDraw(args.vg, pianoSvg[(module->scale[3])  ? 15 :  3 ]->handle);
			nvgTranslate(args.vg, 5, 0);
			svgDraw(args.vg, pianoSvg[(module->scale[4])  ? 16 :  4 ]->handle);
			nvgTranslate(args.vg, 7, 0);
			svgDraw(args.vg, pianoSvg[(module->scale[5])  ? 17 :  5 ]->handle);
			nvgTranslate(args.vg, 6, 0);
			svgDraw(args.vg, pianoSvg[(module->scale[6])  ? 18 :  6 ]->handle);
			nvgTranslate(args.vg, 5, 0);
			svgDraw(args.vg, pianoSvg[(module->scale[7])  ? 19 :  7 ]->handle);
			nvgTranslate(args.vg, 5, 0);
			svgDraw(args.vg, pianoSvg[(module->scale[8])  ? 20 :  8 ]->handle);
			nvgTranslate(args.vg, 5, 0);
			svgDraw(args.vg, pianoSvg[(module->scale[9])  ? 21 :  9 ]->handle);
			nvgTranslate(args.vg, 5, 0);
			svgDraw(args.vg, pianoSvg[(module->scale[10]) ? 22 : 10 ]->handle);
			nvgTranslate(args.vg, 5, 0);
			svgDraw(args.vg, pianoSvg[(module->scale[11]) ? 23 : 11 ]->handle);
			nvgRestore(args.vg);
		
			// 11 character display
			nvgSave(args.vg);
			nvgTranslate(args.vg, 0, 11);
			std::string lcdText = module->lcdText;
			lcdText.append(11, ' '); // Ensure the string is long enough
			for (int i = 0; i < 11; i++) {
				char c = lcdText.at(i);
				svgDraw(args.vg, asciiSvg[ c - 32 ]->handle);
				nvgTranslate(args.vg, 6, 0);
			}
			nvgRestore(args.vg);
		}
	}
}; // LCDDrawWidgetPiano
