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
// Other than that, this widget library is unencumbered by the licensing restrictions of
// VCV's component library, as it doesn't use its graphics or call its code.
// 
// Widgets are only added to my library as the need arises. If I'm not using a specific variant,
// I do not create it until I need it.
//
// If you want to re-use one of my one-off widgets that is not in this file but in a module, thus
// covered by the GPL, and wish to receive the code of that widget under the WTFPL, contact me.
// 
// Most of the SVG files used in this file are also distributed under the WTFPL,
// see the individual LICENSE files in folders for exceptions (such as Arcane's CC-BY-NC graphics).
// If you re-use them, please change my signature color scheme to your own.
// It's easy to change colors in bulk using search and replace in a text editor. 
// This request is not legally binding to keep licensing rules simple.



#pragma once
using namespace rack;
extern Plugin* pluginInstance;


namespace W { // I don't want to type MyCoolPersonalWidgets:: every damn time, thank you


/* --------------------------------------------------------------------------------------------- */
/* ---- Base ----------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------------- */


// All the features, none of the shade
struct SvgSwitchUnshadowed : SvgSwitch {
    SvgSwitchUnshadowed() {
        fb = new widget::FramebufferWidget;
        addChild(fb);

        sw = new widget::SvgWidget;
        fb->addChild(sw);
    }
};


// This is essentially a SvgWidget that inherits from LightWidget instead,
// for Lights Off support. 
struct LitSvgWidget : LightWidget {
    FramebufferWidget *fb;
	std::shared_ptr<Svg> svg;
    bool hidden = false;

    void wrap() {
        if (svg && svg->handle) {
            box.size = math::Vec(svg->handle->width, svg->handle->height);
        }
        else {
            box.size = math::Vec();
        }
    }

    void setSvg(std::shared_ptr<Svg> svg) {
        this->svg = svg;
        hidden = false;
        wrap();
    }

    void hide() {
        hidden = true;
    }

    void show() {
        hidden = false;
    }

    void draw(const DrawArgs& args) override {
        if (svg && svg->handle && !hidden) {
            svgDraw(args.vg, svg->handle);
        }
    }
};


// This is a SvgSwitch with special support for Lights Off: every frame past the first
// is displayed as a lit overlay, while the first frame is constantly displayed unlit.
struct LitSvgSwitch : Switch {
	FramebufferWidget* fb;
	CircularShadow* shadow;
	SvgWidget* sw;
    LitSvgWidget* lsw;
	std::vector<std::shared_ptr<Svg>> frames;

	LitSvgSwitch()  {
        fb = new FramebufferWidget;
        addChild(fb);

        shadow = new CircularShadow;
        fb->addChild(shadow);
        shadow->box.size = math::Vec();

        sw = new SvgWidget;
        fb->addChild(sw);

        lsw = new LitSvgWidget;
        fb->addChild(lsw);
    }

	/** Adds an SVG file to represent the next switch position */
	void addFrame(std::shared_ptr<Svg> svg) {
        frames.push_back(svg);
        // If this is our first frame, automatically set SVG and size
        if (!sw->svg) {
            sw->setSvg(svg);
            box.size = sw->box.size;
            lsw->box.size = sw->box.size;
            fb->box.size = sw->box.size;
            // Move shadow downward by 10%
            shadow->box.size = sw->box.size;
            shadow->box.pos = math::Vec(0, sw->box.size.y * 0.10);
        }
    }

	void onChange(const event::Change& e) override {
        ParamQuantity* const paramQuantity = getParamQuantity();
        if (!frames.empty() && paramQuantity) {
            int index = (int) std::round(paramQuantity->getValue() - paramQuantity->getMinValue());
            index = math::clamp(index, 0, (int) frames.size() - 1);
            sw->setSvg(frames[0]);
            if (index > 0) {
                lsw->setSvg(frames[index]);
            } else {
                lsw->hide();
            }
            fb->dirty = true;
        }
        ParamWidget::onChange(e);
    }
 
};


// This is a SvgSwitch with special support for Lights Off: every frame past the first
// is displayed as a lit overlay, while the first frame is constantly displayed unlit.
// This version has no shadow.
struct LitSvgSwitchUnshadowed : Switch {
	FramebufferWidget* fb;
	SvgWidget* sw;
    LitSvgWidget* lsw;
	std::vector<std::shared_ptr<Svg>> frames;

	LitSvgSwitchUnshadowed()  {
        fb = new FramebufferWidget;
        addChild(fb);

        sw = new SvgWidget;
        fb->addChild(sw);

        lsw = new LitSvgWidget;
        fb->addChild(lsw);
    }

	/** Adds an SVG file to represent the next switch position */
	void addFrame(std::shared_ptr<Svg> svg) {
        frames.push_back(svg);
        // If this is our first frame, automatically set SVG and size
        if (!sw->svg) {
            sw->setSvg(svg);
            box.size = sw->box.size;
            lsw->box.size = sw->box.size;
            fb->box.size = sw->box.size;
        }
    }

	void onChange(const event::Change& e) override {
        ParamQuantity* const paramQuantity = getParamQuantity();
        if (!frames.empty() && paramQuantity) {
            int index = (int) std::round(paramQuantity->getValue() - paramQuantity->getMinValue());
            index = math::clamp(index, 0, (int) frames.size() - 1);
            sw->setSvg(frames[0]);
            if (index > 0) {
                lsw->setSvg(frames[index]);
            } else {
                lsw->hide();
            }
            fb->dirty = true;
        }
        ParamWidget::onChange(e);
    }
 
};


/* --------------------------------------------------------------------------------------------- */
/* ---- Lights for Jacks ----------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------------- */

// Values are kinda yolo'd by trial and error here.
// We don't want a halo - they're too visible on my faceplates, and slated for removal in VCV 2.0 anyway


// Those lights are used for dynamic jacks.
// They must be added before transparent jacks, at the same position (use the helper).
// They are cut off in the middle for Lights Off compatibility.
struct JackLight : app::ModuleLightWidget {
    JackLight() {
        this->box.size = mm2px(math::Vec(8.0, 8.0));
        this->bgColor = nvgRGB(0x0e, 0x69, 0x77);
    }
    
    void drawLayer(const DrawArgs& args, int layer) override {
        if (layer != 1)
            return;

        float radius = std::min(this->box.size.x, this->box.size.y) / 2.0 - 0.5f;
        float holeRadius = mm2px(3.f);

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


struct JackDynamicLightInput : JackLight {
    JackDynamicLightInput() {
        this->addBaseColor(nvgRGB(0xff, 0xcc, 0x03));
    }
};


struct JackDynamicLightOutput : JackLight {
    JackDynamicLightOutput() {
        this->addBaseColor(nvgRGB(0xfc, 0xae, 0xbb));
    }
};


// Those lights are used for static jacks.
// They must be added before transparent jacks, at the same position (use the helper).
// They are cut off in the middle for Lights Off compatibility.
struct JackStaticLight : app::LightWidget {
    JackStaticLight() {
        this->box.size = mm2px(math::Vec(8.0, 8.0));
    }
    
    void drawLayer(const DrawArgs& args, int layer) override {
        if (layer != 1)
            return;

        float radius = std::min(this->box.size.x, this->box.size.y) / 2.0 - 0.5f;
        float holeRadius = mm2px(3.f);

        nvgBeginPath(args.vg);
        nvgCircle(args.vg, radius + 1.f, radius + 1.f, radius);
        nvgCircle(args.vg, radius + 1.f, radius + 1.f, holeRadius);
        nvgPathWinding(args.vg, NVG_HOLE);

        // Foreground
        nvgFillColor(args.vg, this->color);
        nvgFill(args.vg);
    }

};


struct JackStaticLightInput : JackStaticLight {
    JackStaticLightInput() {
        this->color = nvgRGB(0xff, 0xcc, 0x03);
    }
};


struct JackStaticLightOutput : JackStaticLight {
    JackStaticLightOutput() {
        this->color = nvgRGB(0xfc, 0xae, 0xbb);
    }
};


// Helper to create a LED that goes behind a static input Jack. The light is constantly lit.
inline JackStaticLightInput* createStaticLightInput(math::Vec pos) {
    JackStaticLightInput* light = new JackStaticLightInput;
    light->box.pos = pos;
	return light;
}


// Helper to create a LED that goes behind a static output Jack. The light is constantly lit.
inline JackStaticLightOutput* createStaticLightOutput(math::Vec pos) {
    JackStaticLightOutput* light = new JackStaticLightOutput;
    light->box.pos = pos;
	return light;
}


// Helper to create a LED that goes behind a dynamically lit input Jack.
inline JackDynamicLightInput* createDynamicLightInput(math::Vec pos, engine::Module* module, int lightId) {
    JackDynamicLightInput* light = new JackDynamicLightInput;
	light->module = module;
	light->firstLightId = lightId;
    light->box.pos = pos;
	return light;
}


// Helper to create a LED that goes behind a dynamically lit output Jack.
inline JackDynamicLightOutput* createDynamicLightOutput(math::Vec pos, engine::Module* module, int lightId) {
    JackDynamicLightOutput* light = new JackDynamicLightOutput;
	light->module = module;
	light->firstLightId = lightId;
    light->box.pos = pos;
	return light;
}




/* --------------------------------------------------------------------------------------------- */
/* ---- Lights for Knobs ----------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------------- */



// Those lights must be added before transparent knobs, at the same position.
// If given a ParamQuantity pointer, draws a little dark segment for Lights Off mode.
struct KnobLight : ModuleLightWidget {
    ParamQuantity* paramQuantity = NULL;
    float min = 0.f;
    float max = 10.f;

    KnobLight() {
        this->box.size = mm2px(math::Vec(8.0f, 8.0f));
        this->bgColor = nvgRGB(0x0e, 0x69, 0x77);
    }
    
    void drawLayer(const DrawArgs& args, int layer) override {
        if (layer != 1)
            return;

        float radius = std::min(this->box.size.x, this->box.size.y) / 2.0 - 2.6f;

        nvgBeginPath(args.vg);
        nvgCircle(args.vg, radius + 2.6f, radius + 2.6f, radius);

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

        // Draw a dark segment to show the position of the knob, for Lights Off support. 
        if (module && paramQuantity) {
            nvgBeginPath(args.vg);
            nvgMoveTo(args.vg, mm2px(4.f), mm2px(4.f));
            // Rotates by -90 degrees in radians
            float value = rescale(paramQuantity->getValue(), min, max, -0.83f * M_PI - 1.570796f, 0.83f * M_PI - 1.570796f);
            float targetX = mm2px(4.f + 3.2f * cos(value));
            float targetY = mm2px(4.f + 3.2f * sin(value));
            nvgLineTo(args.vg, targetX, targetY);
            nvgStrokeColor(args.vg, nvgRGB(0x00, 0x00, 0x00));
            nvgStrokeWidth(args.vg, 2.f);
            nvgStroke(args.vg);
        }        
    }

};


// Helper to create a KnobLight that goes below the knob, with a little dark segment for Lights Off mode.
template <class TKnobLight>
TKnobLight* createKnobLight(math::Vec pos, Module* module, int lightId, int paramId, float min, float max) {
    TKnobLight* o = new TKnobLight;
    o->box.pos = pos;
    o->module = module;
    o->firstLightId = lightId;
    if (module) o->paramQuantity = module->paramQuantities[paramId];
    o->min = min;
    o->max = max;
    return o;
}

struct KnobLightYellow : KnobLight {
    KnobLightYellow() {
        this->addBaseColor(nvgRGB(0xff, 0xcc, 0x03));
    }
};




/* --------------------------------------------------------------------------------------------- */
/* ---- Other Lights --------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------------- */


// Tiny little status lights. 2.17mm
struct StatusLight : ModuleLightWidget {
    StatusLight() {
        this->box.size = mm2px(math::Vec(2.176f, 2.176f));
        this->bgColor = nvgRGB(0x0e, 0x69, 0x77);
        this->borderColor = nvgRGB(0x08, 0x3d, 0x45);
    }
    
    void drawLayer(const DrawArgs& args, int layer) override {
        if (layer != 1)
            return;

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
            nvgStrokeWidth(args.vg, mm2px(0.2));
            nvgStrokeColor(args.vg, this->borderColor);
            nvgStroke(args.vg);
        }
    }

};


// 2.17mm
struct StatusLightOutput : StatusLight {
    StatusLightOutput() {
        this->addBaseColor(nvgRGB(0xfc, 0xae, 0xbb));
    }
};


// 2.17mm
struct StatusLightInput : StatusLight {
    StatusLightInput() {
        this->addBaseColor(nvgRGB(0xff, 0xcc, 0x03));
    }
};




/* --------------------------------------------------------------------------------------------- */
/* ---- Jacks ---------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------------- */


// Transparent jacks are shown above a light.
struct JackTransparent : SVGPort {
    JackTransparent() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/jack-transparent.svg")));
    }
};



/* --------------------------------------------------------------------------------------------- */
/* ---- Switches ------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------------- */

// ------------------------- Pushbuttons ----------------------------------------------------------

// 5mm
struct SmallButton : LitSvgSwitch {
    SmallButton() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-500-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-500-on.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-500-pink.svg")));
    }
};


// 5mm
struct SmallButtonMomentary : LitSvgSwitch {
    SmallButtonMomentary() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-500-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-500-on.svg")));
        momentary = true;
    }
};


// 7mm
struct ReducedButton : LitSvgSwitch {
    ReducedButton() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-700-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-700-on.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-700-pink.svg")));
    }
};


// 8.20mm.
struct Button : LitSvgSwitch {
    Button() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-on.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-pink.svg")));
    }
};


// 8.20mm.
struct ButtonMomentary : LitSvgSwitch {
    ButtonMomentary() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-on.svg")));
        momentary = true;
    }
};


// 8.20mm. You won't guess its color when you press it.
struct ButtonPink : LitSvgSwitch {
    ButtonPink() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-820-pink.svg")));
    }
};



// ------------------------- Rocker switches ------------------------------------------------------


// Rocker siwtch, horizontal. Left is default
struct RockerSwitchHorizontal : SvgSwitchUnshadowed {
    RockerSwitchHorizontal() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/rocker-switch-800-l.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/rocker-switch-800-r.svg")));
    }
};


// Rocker siwtch, vertical. Up is default
struct RockerSwitchVertical : SvgSwitchUnshadowed {
    RockerSwitchVertical() {
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
    }
};

// 8.2mm
struct KnobTransparent : Knob {
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


// My personal brand, featuring the Cool S. Standard vertical position is 114.5mm. It's 13.6mm wide.
// Using a SvgScrew for the handy built-in framebuffer.
// If you reuse these components, change the corresponding SVG file. Do not reuse my signature in your own works.
// See the README for full legal details. 
struct Signature : SvgScrew {
    Signature() {
        setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/signature/signature.svg")));
    }
};


/* --------------------------------------------------------------------------------------------- */
/* ---- Custom ModuleWidget -------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------------- */


struct ModuleWidget : app::ModuleWidget {

    void addStaticInput(math::Vec pos, engine::Module* module, int inputId) {
        addChild(W::createStaticLightInput(pos));
        addInput(rack::createInput<W::JackTransparent>(pos, module, inputId));
    }

    void addStaticOutput(math::Vec pos, engine::Module* module, int outputId) {
        addChild(W::createStaticLightOutput(pos));
        addOutput(rack::createOutput<W::JackTransparent>(pos, module, outputId));
    }

    void addDynamicInput(math::Vec pos, engine::Module* module, int inputId, int lightId) {
        if (module) addChild(W::createDynamicLightInput(pos, module, lightId));
        addInput(rack::createInput<W::JackTransparent>(pos, module, inputId));
    }

    void addDynamicOutput(math::Vec pos, engine::Module* module, int outputId, int lightId) {
        if (module) addChild(W::createDynamicLightOutput(pos, module, lightId));
        addOutput(rack::createOutput<W::JackTransparent>(pos, module, outputId));
    }
    
};


} // namespace W
