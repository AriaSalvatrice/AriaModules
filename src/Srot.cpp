/*  Copyright (C) 2019-2020 Aria Salvatrice
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

/* Srot should do the following:
- Sort ascending
- Sort descending
- Sort ascending absolute value
- Sort descending absolute value
- Rotate by N channels
- Sort randomly on trigger
- Trim to _n_ outputs |
- Pad to _n_ outputs  | Min max
- 

*/
#include "plugin.hpp"

namespace Srot {

struct Srot : Module {
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        NUM_INPUTS
    };
    enum OutputIds {
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };
    

    Srot() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    }
    
    void process(const ProcessArgs& args) override {

    }

};


struct SrotWidget : ModuleWidget {
    SrotWidget(Srot* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/faceplates/Srot.svg")));
        
        // Signature 
        addChild(createWidget<AriaSignature>(mm2px(Vec(5.899, 114.538))));

        // Screws
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        
    }

};

} // namespace Srot

Model* modelSrot = createModel<Srot::Srot, Srot::SrotWidget>("Srot");
