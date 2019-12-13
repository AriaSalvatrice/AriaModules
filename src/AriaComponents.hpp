#pragma once
#include "rack.hpp"

using namespace rack;

extern Plugin* pluginInstance;

struct AriaScrew : SvgScrew {
	AriaScrew() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/screw.svg")));
	}
};

// FIXME - Using the screw code might be a bad idea, but it works.
struct AriaSignature : SvgScrew {
	AriaSignature() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/signature.svg")));
	}
};

struct AriaJackIn : SVGPort {
	AriaJackIn() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/jack-in.svg")));
	}
};

struct AriaJackOut : SVGPort {
	AriaJackOut() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/jack-out.svg")));
	}
};