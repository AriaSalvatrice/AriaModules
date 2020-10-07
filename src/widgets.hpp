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

// NOTE: My signature SVG graphic is copyrighted. If you reuse my components, remove my signature.

// NOTE: Widgets are only added to my library as the need arises. If I'm not using a specific variant,
// I do not create it until I need it. And if I need it only once, I create it in tne module instead.
// Widgets are only moved to my library once used in more than a single module.

// NOTE: If you want to re-use one of my one-off widgets that is not in this file but in a module, thus
// covered by the GPL, and wish to receive its code under the WTFPL, contact me.



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


// Those lights must be added before transparent jacks, at the same position.
// They are cut off in the middle for Lights Off compatibility.
// Values are kinda yolo'd by trial and error here.
struct WidgetLight : app::ModuleLightWidget {
    WidgetLight() {
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
};


struct InputWidgetLight : WidgetLight {
    InputWidgetLight() {
        this->addBaseColor(nvgRGB(0xff, 0xcc, 0x03));
    }
};


struct OutputWidgetLight : WidgetLight {
    OutputWidgetLight() {
        this->addBaseColor(nvgRGB(0xfc, 0xae, 0xbb));
    }
};




/* --------------------------------------------------------------------------------------------- */
/* ---- Jacks ---------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------------- */

// TODO: Cut off jack light.
// See https://github.com/david-c14/ModularFungi/issues/15#issuecomment-657193438 how to cut off a light


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
    InputWidgetLight* light = new InputWidgetLight;
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
    OutputWidgetLight* light = new OutputWidgetLight;
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


// ------------------------- Rocker switches ------------------------------------------------------


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
