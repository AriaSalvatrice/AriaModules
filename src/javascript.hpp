/*             DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
                    Version 2, December 2004

 Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>

 Everyone is permitted to copy and distribute verbatim or modified
 copies of this license document, and changing it is allowed as long
 as the name is changed.

            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

  0. You just DO WHAT THE FUCK YOU WANT TO.
*/

// This encapsulates QuickJS, the same Javascript engine used in VCV Prototype.
// This is very experimental, and done in the simplest way that would accomplish my goals,
// since the documentation of quickjs is mostly "Just read the uncommented headers lol".
//
// I plan to keep it this simple until I run into an obvious pain point.
//
// In general, the big idea is to load snippets from javascript-libraries,
// craft a string to call a function and assign it to a variable, 
// read that variable, and use JSON for interchange because eh that's as
// far as I'm willing to figure out this thing.
//
// Spinning up a new runtime on demand is basically instant, but this stuff is
// just to do one-off data processing. Don't go around writing an oscillator with it.
// 
// Jerry Sievert's fork of Quickjs (and advice!) were used: https://github.com/JerrySievert/QuickJS
// See makefile for how to add it to a project.

#pragma once
#include <rack.hpp>

// QuickJS always throws a warning here, but it works.
#include "quickjs.h"

namespace Javascript {

struct Runtime {    
    JSRuntime *runtime = NULL;
    JSContext *context = NULL;
    JSValue argv;
    JSValue globalObject;

    Runtime () {
        runtime = JS_NewRuntime();
        context = JS_NewContext(runtime);

        argv = JS_NewObject(context);
        globalObject = JS_GetGlobalObject(context);
    }

    ~Runtime () {
        JS_FreeValue(context, argv);
        JS_FreeValue(context, globalObject);
        if (context) JS_FreeContext(context);
        if (runtime) JS_FreeRuntime(runtime);
    }

    void evaluateString(std::string script) {
        JSValue evaluatedScript = JS_Eval(context, script.c_str(), script.size(), "Evaluated script", 0);
        JS_FreeValue(context, evaluatedScript);
    }

    const char* readVariableAsChar(const char* variable){
        JSValue value = JS_GetPropertyStr(context, globalObject, variable);
        const char *readValue = JS_ToCString(context, value);
        JS_FreeValue(context, value);
        return readValue;
    }

    int32_t readVariableAsInt32(const char* variable){
        int32_t readValue = 0;
        JSValue value = JS_GetPropertyStr(context, globalObject, variable);
        JS_ToInt32(context, &readValue, value);
        JS_FreeValue(context, value);
        return readValue;
    }


};

} // Javascript
