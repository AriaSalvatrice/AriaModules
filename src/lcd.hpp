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
#pragma once

using namespace rack;
extern Plugin* pluginInstance;

// This LCD widget is only concerned with displaying data. 
// Its size is currently fixed to 36*10mm - 2 lines of 11 characters, could be easily changed.
//
// The LCD PAGE is the layout, e.g., two lines of text, or a piano and a line of text.
// The LCD MODE is the internal state of the LCD as tracked by each module
// individually. This widget doesn't care about the LCD MODE.
// 
// On Arcane and Darius, the SVG of the LCD is a little bit too small to display
// descenders on the second line of text, so uppercase is mostly used.
// Future modules will have a slightly larger LCD to fit descenders if desired.
//
// If you like this widget, it's probably reasonably easy to re-use in your own module. 
// However, it's not very generic. And it's not a good or an idiomatic design, I'm new
// at C++ stuff. I am trying to retrofit older modules to use it more intelligently, too.
// And I have not profiled its performance impact in detail.
//
// To see how it works, analyze Solomon.cpp - older modules do not implement the LCD
// as cleanly. They are slowly being retrofitted to do it better.
// 
// If you're gonna reuse this code despite the warnings, please change my signature
// color scheme to your own. You can recolor the letters in batch with a text editor.

namespace Lcd {

// What to draw on the LCD.
enum LcdPage {
    OFF_PAGE,
    TEXT1_PAGE,
    TEXT2_PAGE,
    TEXT1_AND_TEXT2_PAGE,
    PIANO_AND_TEXT2_PAGE
};

// FIXME: lcdLastInteraction & lcdMode should be moved from other modules.
// The LCD's status.
struct LcdStatus {
    // The first line, not displayed on every page. 
    std::string lcdText1 = "";

    // The second line, currently displayed on every page. 
    std::string lcdText2 = "";

    // The piano display
    std::array<bool, 12> pianoDisplay;

    // Whether to redraw the widget.
    bool lcdDirty = false;

    // LCD-specific page: whether to draw two lines of text, a piano, etc.
    int lcdPage = OFF_PAGE;

    // Module-specific mode: this widget has no knowledge what it means.
    int lcdMode = 0;

    // For any info on a timer in the module. This widget has no knowledge what it means.
    float lcdLastInteraction = 0.f;

    LcdStatus() {
        for (int i = 0; i < 12; i++) pianoDisplay[i] = false;
    }

};

// The framebuffer holding the Draw widget.
template <typename TModule>
struct LcdFramebufferWidget : FramebufferWidget{
    TModule *module;

    LcdFramebufferWidget(TModule *_module){
        module = _module;
    }

    void step() override{
        if (module) { // Required to avoid crashing module browser
            if(module->lcdStatus.lcdDirty){
                FramebufferWidget::dirty = true;
                module->lcdStatus.lcdDirty = false;
            }
            FramebufferWidget::step();
        }
    }
};

// The draw widget.
template <class TModule>
struct LcdDrawWidget : TransparentWidget {
    TModule *module;
    std::array<std::shared_ptr<Svg>, 95> asciiSvg; // 32 to 126, the printable range
    std::array<std::shared_ptr<Svg>, 24> pianoSvg; // 0..11: Unlit, 12..23 = Lit
    std::string lcdText1;
    std::string lcdText2;

    LcdDrawWidget(TModule *_module) {
        this->module = _module;
        if (module) {
            box.size = mm2px(Vec(36.0, 10.0));
            for (int i = 0; i < 12; i++) // Unlit
                pianoSvg[i] = APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/lcd/piano/u" + std::to_string(i) + ".svg"));
            for (int i = 0; i < 12; i++) // Lit
                pianoSvg[i + 12] = APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/lcd/piano/l" + std::to_string(i) + ".svg"));
            for (int i = 0; i < 95; i++)
                asciiSvg[i] = APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/lcd/Fixed_v01/" + std::to_string(i + 32) + ".svg"));
        }
    }

    // Decides what to draw depending on the page.
    void draw(const DrawArgs &args) override {
        if (module) {
            nvgScale(args.vg, 1.5, 1.5);
        
            // Piano display at the top.
            if ( module->lcdStatus.lcdPage == PIANO_AND_TEXT2_PAGE ) {
                nvgSave(args.vg);
                svgDraw(args.vg, pianoSvg[(module->lcdStatus.pianoDisplay[0])  ? 12 :  0 ]->handle);
                nvgTranslate(args.vg, 6, 0);
                svgDraw(args.vg, pianoSvg[(module->lcdStatus.pianoDisplay[1])  ? 13 :  1 ]->handle);
                nvgTranslate(args.vg, 5, 0);
                svgDraw(args.vg, pianoSvg[(module->lcdStatus.pianoDisplay[2])  ? 14 :  2 ]->handle);
                nvgTranslate(args.vg, 5, 0);
                svgDraw(args.vg, pianoSvg[(module->lcdStatus.pianoDisplay[3])  ? 15 :  3 ]->handle);
                nvgTranslate(args.vg, 5, 0);
                svgDraw(args.vg, pianoSvg[(module->lcdStatus.pianoDisplay[4])  ? 16 :  4 ]->handle);
                nvgTranslate(args.vg, 7, 0);
                svgDraw(args.vg, pianoSvg[(module->lcdStatus.pianoDisplay[5])  ? 17 :  5 ]->handle);
                nvgTranslate(args.vg, 6, 0);
                svgDraw(args.vg, pianoSvg[(module->lcdStatus.pianoDisplay[6])  ? 18 :  6 ]->handle);
                nvgTranslate(args.vg, 5, 0);
                svgDraw(args.vg, pianoSvg[(module->lcdStatus.pianoDisplay[7])  ? 19 :  7 ]->handle);
                nvgTranslate(args.vg, 5, 0);
                svgDraw(args.vg, pianoSvg[(module->lcdStatus.pianoDisplay[8])  ? 20 :  8 ]->handle);
                nvgTranslate(args.vg, 5, 0);
                svgDraw(args.vg, pianoSvg[(module->lcdStatus.pianoDisplay[9])  ? 21 :  9 ]->handle);
                nvgTranslate(args.vg, 5, 0);
                svgDraw(args.vg, pianoSvg[(module->lcdStatus.pianoDisplay[10]) ? 22 : 10 ]->handle);
                nvgTranslate(args.vg, 5, 0);
                svgDraw(args.vg, pianoSvg[(module->lcdStatus.pianoDisplay[11]) ? 23 : 11 ]->handle);
                nvgRestore(args.vg);
            }

            // 11 character display at the top.
            if ( module->lcdStatus.lcdPage == TEXT1_PAGE
              || module->lcdStatus.lcdPage == TEXT1_AND_TEXT2_PAGE ) {
                nvgSave(args.vg);
                lcdText1 = module->lcdStatus.lcdText1;
                lcdText1.append(11, ' '); // Ensure the string is long enough
                for (int i = 0; i < 11; i++) {
                    char c = lcdText1.at(i);
                    svgDraw(args.vg, asciiSvg[ c - 32 ]->handle);
                    nvgTranslate(args.vg, 6, 0);
                }
                nvgRestore(args.vg);
            }
        
            // 11 character display at the bottom.
            if ( module->lcdStatus.lcdPage == TEXT2_PAGE
              || module->lcdStatus.lcdPage == TEXT1_AND_TEXT2_PAGE
              || module->lcdStatus.lcdPage == PIANO_AND_TEXT2_PAGE ) {
                nvgSave(args.vg);
                nvgTranslate(args.vg, 0, 11);
                lcdText2 = module->lcdStatus.lcdText2;
                lcdText2.append(11, ' '); // Ensure the string is long enough
                for (int i = 0; i < 11; i++) {
                    char c = lcdText2.at(i);
                    svgDraw(args.vg, asciiSvg[ c - 32 ]->handle);
                    nvgTranslate(args.vg, 6, 0);
                }
                nvgRestore(args.vg);
            }
        }
    }

}; // LcdDrawWidget

template <class TModule>
Lcd::LcdFramebufferWidget<TModule>* createLcd(math::Vec pos, TModule *module) {
    Lcd::LcdFramebufferWidget<TModule> *lfb = new Lcd::LcdFramebufferWidget<TModule>(module);
    Lcd::LcdDrawWidget<TModule> *ldw = new Lcd::LcdDrawWidget<TModule>(module);
    lfb->box.pos = pos;
    lfb->addChild(ldw);
    return lfb;
}

} // Lcd
