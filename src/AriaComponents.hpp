#pragma once

using namespace rack;

extern Plugin* pluginInstance;

//////////////////////////////// Decorative

// These require a standard <3-shaped screwdriver, provided complimentary with every purchasee. 
struct AriaScrew : SvgScrew {
	AriaScrew() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/screw.svg")));
	}
};

// My personal brand, featuring the Cool S.
// FIXME - Using a SVGWidget causes graphical issues I don't understand.
// SvgScrew still causes a few problems with dark patches on the preview.
struct AriaSignature : SvgScrew {
	AriaSignature() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/signature.svg")));
	}
};



//////////////////////////////// Jacks & Jack lights

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

// This is the light that goes behind transparent jacks. Add it before adding the jack.
// Taken from the component library, I don't fully understand how it works.
// TODO: clean this up to remove any dependency on the component library.
// Also, I can most likely merge the light and the widget into one single component (cf. LEDButton)
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

// Set it 0.2mm to the bottom right of the 8.2mm component, or just all centered
template <typename TBase>
struct AriaJackLight : TBase {
	AriaJackLight() {
		this->box.size = app::mm2px(math::Vec(7.8, 7.8));
	}
};





//////////////////////////////// Switches
// FIXME: Remove the underscores for style.

// 5.00mm switch. Yellow when lit.
struct AriaPushButton500 : SvgSwitch {
	AriaPushButton500() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-500-off.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-500-on.svg")));
	}
};

// 7.00mm switch. Samesies.
struct AriaPushButton700 : SvgSwitch {
	AriaPushButton700() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-700-off.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-700-on.svg")));
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
