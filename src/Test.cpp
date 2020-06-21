#include "plugin.hpp"
#include <quickjs/quickjs.h>

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
    JSRuntime *rt = NULL;
    JSContext *ctx = NULL;
    
    Test() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

        rt = JS_NewRuntime();
        ctx = JS_NewContext(rt);

        static const std::string& script = R"(
            function hello() {
                var z = 777;
                return z;
            }

            stuff = "Henlo from Javascript lol";
        )";

        JSValue whocareslol = JS_NewObject(ctx);
        JSValue global_obj = JS_GetGlobalObject(ctx);
        JSValue idek = JS_Eval(ctx, script.c_str(), script.size(), "test", 0);
        JSValue function = JS_GetPropertyStr(ctx, global_obj, "hello");
        JSValue val = JS_Call(ctx, function, JS_UNDEFINED, 1, &whocareslol);
        JSValue stuff = JS_GetPropertyStr(ctx, global_obj, "stuff");

        int32_t num = 0;
        JS_ToInt32(ctx, &num, val);
        size_t plen;
        const char *string = JS_ToCStringLen(ctx, &plen, stuff);
        DEBUG("%s", string);
        DEBUG("Lucky number: %u", num);

        JS_FreeValue(ctx, stuff);
        JS_FreeValue(ctx, val);
        JS_FreeValue(ctx, function);
        JS_FreeValue(ctx, idek);
        JS_FreeValue(ctx, global_obj);
        JS_FreeValue(ctx, whocareslol);
    }

    ~Test(){
        if (ctx) JS_FreeContext(ctx);
        if (rt) JS_FreeRuntime(rt);
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
