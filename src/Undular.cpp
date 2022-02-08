/*  Copyright (C) 2019-2020 Aria Salvatrice
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

// See what changed in Rack/src/app/RackScrollWidget.cpp whenever things break

// RACK_GRID_HEIGHT = 3U   = 380px
// RACK_GRID_WIDTH  = 1hp  =  15px

#include "plugin.hpp"
#include <settings.hpp>

namespace Undular {

// No, I will not debug race conditions with multiple instances
static bool ariaSalvatriceUndularSingletonOwned = false;

const int DIVISION = 32;

struct Undular : Module {
    enum ParamIds {
        PADDING_PARAM,
        X_STEP_PARAM,
        Y_STEP_PARAM,
        X_LOCK_PARAM,
        Y_LOCK_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        U_INPUT,
        D_INPUT,
        L_INPUT,
        R_INPUT,
        X_INPUT,
        Y_INPUT,
        Z_INPUT,
        OPACITY_INPUT,
        TENSION_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        DEBUG_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };
    
    bool owningSingleton = false;
    bool initialized = false;
    bool xActivated = false; // These provide a 1-division lag to snapshot
    bool yActivated = false; // the initial value upon cable connection without
    bool zActivated = false; // acting upon it. 
    bool opacityActivated = false;
    bool tensionActivated = false;
    bool jumpUp = false;
    bool jumpDown = false;
    bool jumpLeft = false;
    bool jumpRight = false;
    bool previousLockX = false;
    bool previousLockY = false;
    float scrollMinX = 0.f;
    float scrollMinY = 0.f;
    float scrollMaxX = 0.f;
    float scrollMaxY = 0.f;
    float lastXInput = 0.f;
    float lastYInput = 0.f;
    float lastZInput = 0.f;
    float lastOpacityInput = 0.f;
    float lastTensionInput = 0.f;
    float lockX = 0.f;
    float lockY = 0.f;
    float startupTimer = 0.f;
    dsp::SchmittTrigger uTrigger;
    dsp::SchmittTrigger dTrigger;
    dsp::SchmittTrigger lTrigger;
    dsp::SchmittTrigger rTrigger;
    dsp::ClockDivider scrollDivider;
    math::Vec position;
    
    ~Undular() { 
        // On destruction, release the singleton. 
        if (owningSingleton) { 
            ariaSalvatriceUndularSingletonOwned = false;
        }
    }
    
    Undular() {
        if (! ariaSalvatriceUndularSingletonOwned) {
            ariaSalvatriceUndularSingletonOwned = true;
            owningSingleton = true;
        }
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(PADDING_PARAM, 0.f, 26.f, 1.f, "Padding", "hp");
        configParam(X_STEP_PARAM, 0.f, 128.f, 32.f, "Horizontal step", "hp");
        configParam(Y_STEP_PARAM, 0.f, 21.f, 3.f, "Vertical step", "U");
        configParam(X_LOCK_PARAM, 0.f, 1.f, 0.f, "Disable manual horizontal scolling");
        configParam(Y_LOCK_PARAM, 0.f, 1.f, 0.f, "Disable manual vertical scolling");
        
        // Feels just as fast as without a divider and saves noticeable CPU
        // NO, reducing this will NOT fix the jitter when locked.
        scrollDivider.setDivision(DIVISION);
    }

    // Do not save the status of the locks
    void onAdd() override{
        params[X_LOCK_PARAM].setValue(0.f);
        params[Y_LOCK_PARAM].setValue(0.f);
    }
    
    // Selects the most useful min & max positions to scroll to. Why do these values work? I DUNNO.
    void updateScrollOffsets() {	
        math::Rect rackScrollBox = APP->scene->rackScroll->box;	
        math::Rect boundingBox = APP->scene->rackScroll->container->getChildrenBoundingBox();
        boundingBox = boundingBox.grow(rackScrollBox.size.mult( - 0.6666)); // This sets left and top offset correctly
        float padding = params[PADDING_PARAM].getValue() * RACK_GRID_WIDTH;

        scrollMinX = boundingBox.pos.x - padding;
        scrollMinY = boundingBox.pos.y - padding;
        scrollMaxX = boundingBox.pos.x + boundingBox.size.x - rackScrollBox.size.x + BND_SCROLLBAR_WIDTH + padding;
        scrollMaxY = boundingBox.pos.y + boundingBox.size.y - rackScrollBox.size.y + BND_SCROLLBAR_HEIGHT + padding;
    }
    
    void processJumpInputs() {
        position = APP->scene->rackScroll->offset;

        jumpUp    = (uTrigger.process(inputs[U_INPUT].getVoltageSum())) ? true : false; 
        jumpDown  = (dTrigger.process(inputs[D_INPUT].getVoltageSum())) ? true : false;
        jumpLeft  = (lTrigger.process(inputs[L_INPUT].getVoltageSum())) ? true : false;
        jumpRight = (rTrigger.process(inputs[R_INPUT].getVoltageSum())) ? true : false;

        // Add a 6U buffer
        if (jumpUp) {
            if (position.y - params[Y_STEP_PARAM].getValue() * RACK_GRID_HEIGHT / 3 > scrollMinY - RACK_GRID_HEIGHT * 2.f) {
                position.y = position.y - params[Y_STEP_PARAM].getValue() * RACK_GRID_HEIGHT / 3;
            } else {
                position.y = scrollMaxY;
            }
            APP->scene->rackScroll->offset = position;
        }
        if (jumpDown) {
            if (position.y + params[Y_STEP_PARAM].getValue() * RACK_GRID_HEIGHT / 3 < scrollMaxY + RACK_GRID_HEIGHT * 2.f) {
                position.y = position.y + params[Y_STEP_PARAM].getValue() * RACK_GRID_HEIGHT / 3;
            } else {
                position.y = scrollMinY;
            }
            APP->scene->rackScroll->offset = position;
        }
        // Add a 32hp buffer
        if (jumpLeft) {
            if (position.x - params[X_STEP_PARAM].getValue() * RACK_GRID_WIDTH > scrollMinX - RACK_GRID_WIDTH * 32) {
                position.x = position.x - params[X_STEP_PARAM].getValue() * RACK_GRID_WIDTH;
            } else {
                position.x = scrollMaxX;
            }
            APP->scene->rackScroll->offset = position;
        }
        if (jumpRight) {
            if (position.x + params[X_STEP_PARAM].getValue() * RACK_GRID_WIDTH / 3 < scrollMaxX + RACK_GRID_WIDTH * 32) {
                position.x = position.x + params[X_STEP_PARAM].getValue() * RACK_GRID_WIDTH;
            } else {
                position.x = scrollMinX;
            }
            APP->scene->rackScroll->offset = position;
        }
    }
    
    // X, Y, Zoom
    void processXYZInputs() {
        position = APP->scene->rackScroll->offset;
        bool positionChanged = false;
        bool zoomChanged = false;
        float newZoom = 0.f;

        if (xActivated and inputs[X_INPUT].isConnected() and inputs[X_INPUT].getVoltage() >= 0.f) {
            if (inputs[X_INPUT].getVoltage() != lastXInput) {
                position.x = scrollMinX + ( (scrollMaxX - scrollMinX) * inputs[X_INPUT].getVoltage() / 10);
                positionChanged = true;
            }
            lastXInput = inputs[X_INPUT].getVoltage();
        }
        
        if (yActivated and inputs[Y_INPUT].isConnected() and inputs[Y_INPUT].getVoltage() >= 0.f) {
            if (inputs[Y_INPUT].getVoltage() != lastYInput) {
                position.y = scrollMinY + ( (scrollMaxY - scrollMinY) * inputs[Y_INPUT].getVoltage() / 10);
                positionChanged = true;
            }
            lastYInput = inputs[Y_INPUT].getVoltage();
        }
        
        if (zActivated and inputs[Z_INPUT].isConnected() and inputs[Z_INPUT].getVoltage() >= 0.f) {
            if (inputs[Z_INPUT].getVoltage() != lastZInput) {
                newZoom = inputs[Z_INPUT].getVoltage() / 2.5f - 2.0f; // Zoom goes from -2.0 to 2.0
                zoomChanged = true;
            }
            lastZInput = inputs[Z_INPUT].getVoltage();
        }
        
        if (inputs[X_INPUT].isConnected()) lastXInput = inputs[X_INPUT].getVoltage();
        if (inputs[Y_INPUT].isConnected()) lastYInput = inputs[Y_INPUT].getVoltage();
        if (inputs[Z_INPUT].isConnected()) lastZInput = inputs[Z_INPUT].getVoltage();
        
        xActivated = (inputs[X_INPUT].isConnected() and initialized) ? true : false;
        yActivated = (inputs[Y_INPUT].isConnected() and initialized) ? true : false;
        zActivated = (inputs[Z_INPUT].isConnected() and initialized) ? true : false;
                
        if (positionChanged and initialized) APP->scene->rackScroll->offset = position;
        if (zoomChanged and initialized) APP->scene->rackScroll->setZoom(std::pow(2.f, newZoom));
    }

    void processCableInputs() {		
        if (inputs[OPACITY_INPUT].isConnected() and inputs[OPACITY_INPUT].getVoltage() >= 0.f and opacityActivated) {
            if (inputs[OPACITY_INPUT].getVoltage() != lastOpacityInput) {
                settings::cableOpacity = math::clamp(inputs[OPACITY_INPUT].getVoltage() / 10.0f, 0.0f, 1.0f);
            }
            lastOpacityInput = inputs[OPACITY_INPUT].getVoltage();
        }
        if (inputs[TENSION_INPUT].isConnected() and inputs[TENSION_INPUT].getVoltage() >= 0.f and tensionActivated) {
            if (inputs[TENSION_INPUT].getVoltage() != lastTensionInput) {
                settings::cableTension = math::clamp(inputs[TENSION_INPUT].getVoltage() / 10.0f, 0.0f, 1.0f);
            }
            lastTensionInput = inputs[TENSION_INPUT].getVoltage();
        }
        opacityActivated = (inputs[OPACITY_INPUT].isConnected()) ? true : false;
        tensionActivated = (inputs[TENSION_INPUT].isConnected()) ? true : false;
    }
    
    void processLocks() {
        // Store values first loop when enabled
        if ( !previousLockX and params[X_LOCK_PARAM].getValue() == 1.f ) lockX = position.x;
        if ( !previousLockY and params[Y_LOCK_PARAM].getValue() == 1.f ) lockY = position.y;
        
        bool positionChanged = false;
        if ( previousLockX 
             and params[X_LOCK_PARAM].getValue() == 1.f 
             and ! inputs[L_INPUT].isConnected()
             and ! inputs[R_INPUT].isConnected()
             and ! inputs[X_INPUT].isConnected() ) {
            position.x = lockX;
            positionChanged = true;	
        }
        if ( previousLockY
             and params[Y_LOCK_PARAM].getValue() == 1.f 
             and ! inputs[U_INPUT].isConnected()
             and ! inputs[D_INPUT].isConnected()
             and ! inputs[Y_INPUT].isConnected() ) {
            position.y = lockY;
            positionChanged = true;	
        }


        if (positionChanged) APP->scene->rackScroll->offset = position;
        
        previousLockX = (params[X_LOCK_PARAM].getValue() == 1.f) ? true : false;
        previousLockY = (params[Y_LOCK_PARAM].getValue() == 1.f) ? true : false;
    }
    
    void process(const ProcessArgs& args) override {
        if (scrollDivider.process()){
            if (owningSingleton) {
                if (startupTimer < (10.f / DIVISION)) {
                    startupTimer += args.sampleTime;
                } else {
                    position = APP->scene->rackScroll->offset;
                    updateScrollOffsets();
                    processJumpInputs();
                    processXYZInputs();
                    processCableInputs();
                    processLocks();
                    initialized = true; // On load, memorize last input but do not act upon it
                }
            }
        }
    }
};


struct AriaPushButtonPadlock820 : W::LitSvgSwitch {
    AriaPushButtonPadlock820() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-padlock-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-padlock-on.svg")));
    }
};


struct UndularWidget : W::ModuleWidget {
    UndularWidget(Undular* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/faceplates/Undular.svg")));
        
        // Signature 
        addChild(createWidget<W::Signature>(mm2px(Vec(5.9f, 114.5f))));

        // Screws
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<W::Screw>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<W::Screw>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        
        // UDLR
        addStaticInput(mm2px(Vec(8.5, 18.0)), module, Undular::U_INPUT);
        addStaticInput(mm2px(Vec(8.5, 30.0)), module, Undular::D_INPUT);
        addStaticInput(mm2px(Vec(2.5, 24.0)), module, Undular::L_INPUT);
        addStaticInput(mm2px(Vec(14.5, 24.0)), module, Undular::R_INPUT);
        
        // Step
        addParam(createParam<W::Knob>(mm2px(Vec(2.6, 36.0)), module, Undular::X_STEP_PARAM));
        addParam(createParam<W::Knob>(mm2px(Vec(14.6, 36.0)), module, Undular::Y_STEP_PARAM));
        
        // Padding
        addParam(createParam<W::Knob>(mm2px(Vec(2.6, 50.2)), module, Undular::PADDING_PARAM));

        // Y & Lock
        addStaticInput(mm2px(Vec(14.5, 50.0)), module, Undular::Y_INPUT);
        addParam(createParam<AriaPushButtonPadlock820>(mm2px(Vec(14.5, 58.5)), module, Undular::Y_LOCK_PARAM));

        // X & Lock
        addStaticInput(mm2px(Vec(2.5, 66.0)), module, Undular::X_INPUT);	
        addParam(createParam<AriaPushButtonPadlock820>(mm2px(Vec(11.0, 66.0)), module, Undular::X_LOCK_PARAM));
        
        // Zoom
        addStaticInput(mm2px(Vec(8.5, 82.0)), module, Undular::Z_INPUT);
        
        // Cables
        addStaticInput(mm2px(Vec(8.5, 95.0)), module, Undular::OPACITY_INPUT);
        addStaticInput(mm2px(Vec(8.5, 103.0)), module, Undular::TENSION_INPUT);

    }
};

} // namespace Undular

Model* modelUndular = createModel<Undular::Undular, Undular::UndularWidget>("Undular");
