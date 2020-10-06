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
// Widgets are only added to my library as the need arises and are pruned if no longer used.
// Widgets are only moved to my library once used in more than a single module.
// If you want to re-use one of my one-off widgets covered by the GPL, but wish to receive
// its code under the WTFPL, contact me. 






// DEPRECATED. This file is slated for complete removal.
// Declarations are removed from this file as I remake them cleaner in widgets.hpp, which contains more maintainable code.






#pragma once

using namespace rack;
extern Plugin* pluginInstance;


/*                          Jacks                       */



// This output jack has a transparent ring, to display a light behind it. 
struct AriaJackTransparent : SVGPort {
    AriaJackTransparent() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/jack-transparent.svg")));
    }
};

/*                          Old style lights - TODO: Remove dependency on component library                       */

template <typename TBase = GrayModuleLightWidget>
struct TOutputLight : TBase {
    TOutputLight() {
        this->addBaseColor(nvgRGB(0xfc, 0xae, 0xbb));
    }
};
typedef TOutputLight<> OutputLight;

template <typename TBase = GrayModuleLightWidget>
struct TInputLight : TBase {
    TInputLight() {
        this->addBaseColor(nvgRGB(0xff, 0xcc, 0x03));
    }
};
typedef TInputLight<> InputLight;


/*                          Jack Lights                       */

// Those lights should be added before transparent jacks, at the same position.
struct AriaJackLight : app::ModuleLightWidget {
    AriaJackLight() {
        this->box.size = app::mm2px(math::Vec(8.0, 8.0));
        this->bgColor = nvgRGB(0x0e, 0x69, 0x77);
        this->borderColor = nvgRGB(0x0e, 0x69, 0x77);
    }
    
    void drawLight(const widget::Widget::DrawArgs& args) override {
        float radius = std::min(this->box.size.x, this->box.size.y) / 2.0;
        nvgBeginPath(args.vg);
        nvgCircle(args.vg, radius, radius, radius);

        // Background
        if (this->bgColor.a > 0.0) {
            nvgFillColor(args.vg, this->bgColor);
            nvgFill(args.vg);
        }

        // Foreground
        if (this->color.a > 0.0) {
            nvgFillColor(args.vg, this->color);
            nvgFill(args.vg);
        }

        // Border
        if (this->borderColor.a > 0.0) {
            nvgStrokeWidth(args.vg, app::mm2px(0.2));
            nvgStrokeColor(args.vg, this->borderColor);
            nvgStroke(args.vg);
        }
    }
};

struct AriaInputLight : AriaJackLight {
    AriaInputLight() {
        this->addBaseColor(nvgRGB(0xff, 0xcc, 0x03));
    }
};

struct AriaOutputLight : AriaJackLight {
    AriaOutputLight() {
        this->addBaseColor(nvgRGB(0xfc, 0xae, 0xbb));
    }
};




/*                          Switches                       */

// 5.00mm switch. Yellow when lit.
struct AriaPushButton500 : SvgSwitch {
    AriaPushButton500() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-500-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-500-on.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-500-pink.svg")));
    }
};

struct AriaPushButton500Momentary : SvgSwitch {
    AriaPushButton500Momentary() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-500-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-500-on.svg")));
        momentary = true;
    }
};

// 7.00mm switch. Samesies.
struct AriaPushButton700 : SvgSwitch {
    AriaPushButton700() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-700-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-700-on.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-700-pink.svg")));
    }
};

struct AriaPushButton700Momentary : SvgSwitch {
    AriaPushButton700Momentary() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-700-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-700-on.svg")));
        momentary = true;
    }
};

// 8.20mm switch. Yes.
struct AriaPushButton820 : SvgSwitch {
    AriaPushButton820() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-on.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-pink.svg")));
    }
};

struct AriaPushButton820Momentary : SvgSwitch {
    AriaPushButton820Momentary() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-on.svg")));
        momentary = true;
    }
};

// You won't guess its color when you press it.
struct AriaPushButton820Pink : SvgSwitch {
    AriaPushButton820Pink() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-pink.svg")));
    }
};


/*                          Knobs                       */

struct AriaKnob820 : app::SvgKnob {
    AriaKnob820() {
        minAngle = -0.83 * M_PI;
        maxAngle = 0.83 * M_PI;
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/knob-820.svg")));
    }
};

struct AriaKnob820Snap : AriaKnob820 {
    AriaKnob820Snap() {
        snap = true;
        AriaKnob820();
    }
};

struct AriaKnob820Transparent : app::SvgKnob {
    AriaKnob820Transparent() {
        minAngle = -0.83 * M_PI;
        maxAngle = 0.83 * M_PI;
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/knob-820-transparent.svg")));
    }
};
