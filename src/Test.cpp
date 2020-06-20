#include "plugin.hpp"
#include <luajit-2.0/lua.hpp>
#include <luajit-2.0/lualib.h>
#include <luajit-2.0/lauxlib.h>


// This module is to make all sorts of tests without having to recompile too much or deal with complex code interactions.

struct Test : Module {
    enum ParamIds {
        ENUMS(TEST_PARAM, 12),
        NUM_PARAMS
    };
    enum InputIds {
        ENUMS(TEST_INPUT, 12),
        NUM_INPUTS
    };
    enum OutputIds {
        ENUMS(TEST_OUTPUT, 12),
        NUM_OUTPUTS
    };
    enum LightIds {
        ENUMS(TEST_LIGHT, 12),
        NUM_LIGHTS
    };
    dsp::ClockDivider testDivider;
    lua_State *L = NULL;
    
    Test() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        testDivider.setDivision(44);
        L = luaL_newstate();
        luaL_openlibs(L);
        luaL_dostring(L, "function add (a , b) return a + b end");
        lua_getglobal(L, "add");
        lua_pushnumber(L, 12);
        lua_pushnumber(L, 46);
        lua_call(L, 2, 1);
        int sum = (int)lua_tonumber(L, -1);
        lua_pop(L, 1);
        DEBUG("Calculation from Lua: %d", sum);
    }

    ~Test(){
        if (L) lua_close(L);
    }

    void process(const ProcessArgs& args) override {

    }
};


struct TestWidget : ModuleWidget {
    TestWidget(Test* module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Test.svg")));
        
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<AriaScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<AriaScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
                
        for (int i = 0; i < 12; i++) {
            addInput(createInput<AriaJackIn>(mm2px(Vec(10.0, 8.0 + i * 10.0)), module, Test::TEST_INPUT + i));
            addOutput(createOutput<AriaJackOut>(mm2px(Vec(20.0, 8.0 + i * 10.0)), module, Test::TEST_OUTPUT + i));
        }

    }
};

Model* modelTest = createModel<Test, TestWidget>("Test");
