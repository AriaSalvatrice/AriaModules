#pragma once
#include <rack.hpp>
#include "AriaComponents.hpp"
// Explicit <array> include required on OS X
#include <array> 

using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

// Split and Merge series
extern Model *modelSplort;
extern Model *modelSmerge;
extern Model *modelSpleet;
extern Model *modelSwerge;
extern Model *modelSplirge;
// extern Model *modelSrot;

// Sequencer
extern Model *modelDarius;

// Arcane
extern Model *modelArcane;
extern Model *modelAtout;
extern Model *modelAleister;

// Live performance
extern Model *modelUndular;

// Blank plate
extern Model *modelBlank;

// extern Model *modelBendlet;
extern Model *modelTest;
