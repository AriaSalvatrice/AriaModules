/*  Copyright (C) 2019-2020 Aria Salvatrice
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/
#pragma once
#include <rack.hpp>
#include "components.hpp"
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

// Quantizers
extern Model *modelQqqq;
extern Model *modelQuack;
extern Model *modelQ;

// Sequencers
extern Model *modelDarius;
extern Model *modelSolomon;

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
