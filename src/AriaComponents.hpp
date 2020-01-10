#pragma once

using namespace rack;

extern Plugin* pluginInstance;

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
// TODO: clean this up to remove any dependency on the component library
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

template <typename TBase>
struct AriaJackLight : TBase {
	AriaJackLight() {
		this->box.size = app::mm2px(math::Vec(7.8, 7.8));
	}
};

// 5.00mm switch. Yellow when lit.
struct AriaPushButton_500 : SvgSwitch {
	AriaPushButton_500() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-500-off.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-500-on.svg")));
	}
};

// 7.00mm switch. Samesies.
struct AriaPushButton_700 : SvgSwitch {
	AriaPushButton_700() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-700-off.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-700-on.svg")));
	}
};