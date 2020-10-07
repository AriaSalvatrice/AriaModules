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

// My signature SVG graphic is copyrighted. If you reuse my components, remove my signature.
//
// Other than that, This widget library is unencumbered by the licensing restrictions of VCV's component library,
// as it doesn't use its graphics or call its code.
// 
// Widgets are only added to my library as the need arises. If I'm not using a specific variant,
// I do not create it until I need it.
//
// If you want to re-use one of my one-off widgets that is not in this file but in a module, thus
// covered by the GPL, and wish to receive its code under the WTFPL, contact me.


// FIXME: Prefix/Suffix

#pragma once
using namespace rack;
extern Plugin* pluginInstance;


namespace W { // I don't want to type MyCoolPersonalWidgets:: every damn time, thank you



/* --------------------------------------------------------------------------------------------- */
/* ---- Base ----------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------------- */
// TODO: Reimplement cleaner, without a shadow, instead of just hiding it.
struct SvgSwitchUnshadowed : SvgSwitch {
    SvgSwitchUnshadowed() {
        shadow->opacity = 0.f;
        SvgSwitch();
    }
};



/* --------------------------------------------------------------------------------------------- */
/* ---- Lights --------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------------- */


// Values are kinda yolo'd by trial and error here.


// Those lights must be added before transparent jacks, at the same position.
// They are cut off in the middle for Lights Off compatibility.
struct JackLight : app::ModuleLightWidget {
    JackLight() {
        this->box.size = app::mm2px(math::Vec(8.0, 8.0));
        this->bgColor = nvgRGB(0x0e, 0x69, 0x77);
    }
    
    void drawLight(const widget::Widget::DrawArgs& args) override {
        float radius = std::min(this->box.size.x, this->box.size.y) / 2.0 - 0.5f;
        float holeRadius = app::mm2px(3.f);

        nvgBeginPath(args.vg);
        nvgCircle(args.vg, radius + 1.f, radius + 1.f, radius);
        nvgCircle(args.vg, radius + 1.f, radius + 1.f, holeRadius);
        nvgPathWinding(args.vg, NVG_HOLE);

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
    }

    void drawHalo(const DrawArgs& args) override {
        // We don't want a halo - they're too visible on my faceplates, and slated for removal in VCV 2.0 anyway
    }

};


// Those lights must be added before transparent knobs, at the same position.
struct KnobLight : ModuleLightWidget {
    KnobLight() {
        this->box.size = app::mm2px(math::Vec(8.0f, 8.0f));
        this->bgColor = nvgRGB(0x0e, 0x69, 0x77);
    }
    
    void drawLight(const widget::Widget::DrawArgs& args) override {
        float radius = std::min(this->box.size.x, this->box.size.y) / 2.0 - 0.5f;

        nvgBeginPath(args.vg);
        nvgCircle(args.vg, radius + 1.f, radius + 1.f, radius);

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
    }

    void drawHalo(const DrawArgs& args) override {
        // We don't want a halo - they're too visible on my faceplates, and slated for removal in VCV 2.0 anyway
    }

};


struct InputJackLight : JackLight {
    InputJackLight() {
        this->addBaseColor(nvgRGB(0xff, 0xcc, 0x03));
    }
};


struct OutputJackLight : JackLight {
    OutputJackLight() {
        this->addBaseColor(nvgRGB(0xfc, 0xae, 0xbb));
    }
};

struct YellowKnobLight : KnobLight {
    YellowKnobLight() {
        this->addBaseColor(nvgRGB(0xff, 0xcc, 0x03));
    }
};


///////////////


// Those lights must be added before transparent knobs, at the same position.
struct StatusLight : ModuleLightWidget {
    StatusLight() {
        this->box.size = app::mm2px(math::Vec(2.176f, 2.176f));
        this->bgColor = nvgRGB(0x0e, 0x69, 0x77);
        this->borderColor = nvgRGB(0x08, 0x3d, 0x45);
    }
    
    void drawLight(const widget::Widget::DrawArgs& args) override {
        float radius = std::min(this->box.size.x, this->box.size.y) / 2.0 - 0.5f;

        nvgBeginPath(args.vg);
        nvgCircle(args.vg, radius, radius , radius);

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

    void drawHalo(const DrawArgs& args) override {
        // We don't want a halo - they're too visible on my faceplates, and slated for removal in VCV 2.0 anyway
    }
};


struct OutputStatusLight : StatusLight {
    OutputStatusLight() {
        this->addBaseColor(nvgRGB(0xfc, 0xae, 0xbb));
    }
};


struct InputStatusLight : StatusLight {
    InputStatusLight() {
        this->addBaseColor(nvgRGB(0xff, 0xcc, 0x03));
    }
};




/* --------------------------------------------------------------------------------------------- */
/* ---- Jacks ---------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------------- */

// TODO: Make all my jacks usee a light widget, including the non-dynamic ones, for Lights Off consistency.

// Base jack is a SVGPort without customizations.
struct Jack : SVGPort {

};


// Non-dynamic input jacks are constantly lit yellow.
struct JackIn : Jack {
    JackIn() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/jack-in.svg")));
        Jack();
    }
};


// Non-dynamic output jacks are constantly lit pink.
struct JackOut : Jack {
    JackOut() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/jack-out.svg")));
        Jack();
    }
};


// Transparent jacks are shown above a light.
struct JackTransparent : Jack {
    JackTransparent() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/jack-transparent.svg")));
        Jack();
    }
};


// Helper to create a lit input comprised of a LED and a transparent Jack
inline Widget* createLitInput(math::Vec pos, engine::Module* module, int inputId, int firstLightId) {
	Widget* o = new Widget;
    InputJackLight* light = new InputJackLight;
    JackTransparent* jack = new JackTransparent;

	light->module = module;
	light->firstLightId = firstLightId;

	jack->module = module;
	jack->type = app::PortWidget::INPUT;
	jack->portId = inputId;

    o->box.pos = pos;
    o->addChild(light);
    o->addChild(jack);
	return o;
}


// Helper to create a lit output comprised of a LED and a transparent Jack
inline Widget* createLitOutput(math::Vec pos, engine::Module* module, int outputId, int firstLightId) {
	Widget* o = new Widget;
    OutputJackLight* light = new OutputJackLight;
    JackTransparent* jack = new JackTransparent;

	light->module = module;
	light->firstLightId = firstLightId;

	jack->module = module;
	jack->type = app::PortWidget::OUTPUT;
	jack->portId = outputId;

    o->box.pos = pos;
    o->addChild(light);
    o->addChild(jack);
	return o;
}




/* --------------------------------------------------------------------------------------------- */
/* ---- Switches ------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------------- */

// ------------------------- Pushbuttons ----------------------------------------------------------

// 5mm
struct SmallButton : SvgSwitch {
    SmallButton() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-500-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-500-on.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-500-pink.svg")));
    }
};


// 5mm
struct SmallButtonMomentary : SvgSwitch {
    SmallButtonMomentary() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-500-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-500-on.svg")));
        momentary = true;
    }
};


// 7mm
struct ReducedButton : SvgSwitch {
    ReducedButton() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-700-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-700-on.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-700-pink.svg")));
    }
};


// 8.20mm.
struct Button : SvgSwitch {
    Button() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-on.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-pink.svg")));
    }
};


// 8.20mm.
struct ButtonMomentary : SvgSwitch {
    ButtonMomentary() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-on.svg")));
        momentary = true;
    }
};


// 8.20mm. You won't guess its color when you press it.
struct ButtonPink : SvgSwitch {
    ButtonPink() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-pink.svg")));
    }
};



// ------------------------- Rocker switches ------------------------------------------------------


// Rocker siwtch, horizontal. Left is default
struct RockerSwitchHorizontal800 : SvgSwitchUnshadowed {
    RockerSwitchHorizontal800() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/rocker-switch-800-l.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/rocker-switch-800-r.svg")));
    }
};


// Rocker siwtch, vertical. Up is default
struct RockerSwitchVertical800 : SvgSwitchUnshadowed {
    RockerSwitchVertical800() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/rocker-switch-800-u.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/rocker-switch-800-d.svg")));
    }
};





/* --------------------------------------------------------------------------------------------- */
/* ---- Knobs ---------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------------- */


// 8.2mm
struct Knob : app::SvgKnob {
    Knob() {
        minAngle = -0.83 * M_PI;
        maxAngle = 0.83 * M_PI;
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/knob-820.svg")));
    }
};

// 8.2mm
struct KnobSnap : Knob {
    KnobSnap() {
        snap = true;
        Knob();
    }
};

// 8.2mm
struct KnobTransparent : app::SvgKnob {
    KnobTransparent() {
        minAngle = -0.83 * M_PI;
        maxAngle = 0.83 * M_PI;
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/knob-820-transparent.svg")));
    }
};




/* --------------------------------------------------------------------------------------------- */
/* ---- Decorative ----------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------------- */

// These require a standard <3-shaped screwdriver, provided complimentary with every purchasee. 
struct Screw : SvgScrew {
    Screw() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/screw.svg")));
    }
};


// My personal brand, featuring the Cool S. Standard vertical position is 114.5mm.
// Using a SvgScrew for the handy built-in framebuffer.
// If you reuse this components, change the corresponding SVG file. Do not reuse my signature in your own works.
// See the README for the full legal details. 
struct Signature : SvgScrew {
    Signature() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/signature/signature.svg")));
    }
};





} // namespace W
