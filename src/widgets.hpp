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
namespace W { // I don't want to type Widgets:: every damn time, thank you




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




/* --------------------------------------------------------------------------------------------- */
/* ---- Jacks ---------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------------- */

// Base jack is a SVGPort without customizations.
struct Jack : SVGPort {

};

// Dynamic jacks can be toggled.
// TODO: Add a light (that is cut off) or an overlay with dynamic opacity.
// See https://github.com/david-c14/ModularFungi/issues/15#issuecomment-657193438 how to cut off a light
struct DJack : Jack {
    void draw(const DrawArgs& args) override {
        Jack::draw(args);
    }
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

// Dynamic input jacks are constantly lit yellow when active.
struct DJackIn : DJack {

};

// TODO: Make a helper to create a dynamic light





/* --------------------------------------------------------------------------------------------- */
/* ---- Switches ------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------------- */




/* --------------------------------------------------------------------------------------------- */
/* ---- Knobs ---------------------------------------------------------------------------------- */
/* --------------------------------------------------------------------------------------------- */




} // Namespace W
