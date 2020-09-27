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

// Widgets are only added to my library as the need arises.
// They are pruned if no longer used.
// Widgets are only moved to my library once used in more than a single module.
// If you want to re-use one of my one-off widgets covered by the GPL, but wish to receive
// its code under the WTFPL, contact me.

#pragma once

using namespace rack;
extern Plugin* pluginInstance;

namespace W {

// - TODO: Remove every single dependency on the component library, since it is not open-source. 
// - TODO: Namespace instead of prefix



/*                          Base                       */

struct SvgSwitchUnshadowed : SvgSwitch {
    SvgSwitchUnshadowed() {
        shadow->opacity = 0.f;
        SvgSwitch();
    }
};


/*                          Decorative                       */

// These require a standard <3-shaped screwdriver, provided complimentary with every purchasee. 
struct Screw : SvgScrew {
    Screw() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/screw.svg")));
    }
};

// My personal brand, featuring the Cool S.
// If you reuse those components, change this SVG file. Do not reuse my signature in your own works.
// See the README for the full details. 
struct Signature : SvgWidget {
    Signature() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/signature/signature.svg")));
    }
};



/*                          Jacks                       */

// Input jacks are always lit yellow.
struct JackIn : SVGPort {
    JackIn() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/jack-in.svg")));
    }
};

// This output jack is always lit pink.
struct JackOut : SVGPort {
    JackOut() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/jack-out.svg")));
    }
};

// This output jack has a transparent ring, to display a light behind it. 
struct JackTransparent : SVGPort {
    JackTransparent() {
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
struct JackLight : app::ModuleLightWidget {
    JackLight() {
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

// These don't build properly but are slated for removal anyway.

// struct InputLight : JackLight {
//     InputLight() {
//         this->addBaseColor(nvgRGB(0xff, 0xcc, 0x03));
//     }
// };

// struct OutputLight : JackLight {
//     OutputLight() {
//         this->addBaseColor(nvgRGB(0xfc, 0xae, 0xbb));
//     }
// };




/*                          Switches                       */

// 5.00mm switch. Yellow when lit.
struct PushButton500 : SvgSwitch {
    PushButton500() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-500-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-500-on.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-500-pink.svg")));
    }
};

struct PushButton500Momentary : SvgSwitch {
    PushButton500Momentary() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-500-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-500-on.svg")));
        momentary = true;
    }
};

// 7.00mm switch. Samesies.
struct PushButton700 : SvgSwitch {
    PushButton700() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-700-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-700-on.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-700-pink.svg")));
    }
};

struct PushButton700Momentary : SvgSwitch {
    PushButton700Momentary() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-700-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-700-on.svg")));
        momentary = true;
    }
};

// 8.20mm switch. Yes.
struct PushButton820 : SvgSwitch {
    PushButton820() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-on.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-pink.svg")));
    }
};

struct PushButton820Momentary : SvgSwitch {
    PushButton820Momentary() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-on.svg")));
        momentary = true;
    }
};

// You won't guess its color when you press it.
struct PushButton820Pink : SvgSwitch {
    PushButton820Pink() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-pink.svg")));
    }
};

// Rocker siwtch, horizontal. Left is default
struct RockerSwitchHorizontal800 : SvgSwitchUnshadowed {
    RockerSwitchHorizontal800() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/rocker-switch-800-l.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/rocker-switch-800-r.svg")));
    }
};

// Rocker siwtch, horizontal. Right is default
struct RockerSwitchHorizontal800Flipped : SvgSwitchUnshadowed {
    RockerSwitchHorizontal800Flipped() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/rocker-switch-800-r.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/rocker-switch-800-l.svg")));
    }
};

struct RockerSwitchVertical800 : SvgSwitchUnshadowed {
    RockerSwitchVertical800() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/rocker-switch-800-u.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/rocker-switch-800-d.svg")));
    }
};


/*                          Knobs                       */

struct Knob820 : app::SvgKnob {
    Knob820() {
        minAngle = -0.83 * M_PI;
        maxAngle = 0.83 * M_PI;
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/knob-820.svg")));
    }
};

struct Knob820Snap : Knob820 {
    Knob820Snap() {
        snap = true;
        Knob820();
    }
};

struct Knob820Transparent : app::SvgKnob {
    Knob820Transparent() {
        minAngle = -0.83 * M_PI;
        maxAngle = 0.83 * M_PI;
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/knob-820-transparent.svg")));
    }
};

} // namespace W
