#pragma once
#include <rack.hpp>
#include "AriaComponents.hpp"
// Explicit <array> include required on OS X
#include <array> 

// Adds unlabeled debug jacks (that do nothing but can be wired as needed during development) to the bottom of modules
// #define ARIA_DEBUG

// Builds modules still under development and not ready for general use.
// Remember to add them to the plugin.json to see them in the browser!
// #define ARIA_DEV_MODULES

using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

// Declare each Model, defined in each module source file

// Split and Merge series
extern Model *modelSplort;
extern Model *modelSmerge;
extern Model *modelSpleet;
extern Model *modelSwerge;
extern Model *modelSplirge;

#ifdef ARIA_DEV_MODULES
// Bend series
extern Model *modelBendlet;
#endif

/* JSON to enable Bendlet:

    {
      "slug": "Bendlet",
      "name": "Bendlet",
      "description": "Pitchbend helper",
      "tags": [
		"Controller",
        "Polyphonic",
        "Utility",
		"Tuner"
      ]
    },
	
*/

// Blank plate
extern Model *modelBlank;