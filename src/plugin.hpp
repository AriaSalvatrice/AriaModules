#pragma once

#include "AriaComponents.hpp"

#include <rack.hpp>


using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;


// Testing a thing
//struct _Screw : SvgScrew {
	//_Screw() {
		//setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/screw.svg")));
//	}
//};



// Declare each Model, defined in each module source file
extern Model *modelSplirge;