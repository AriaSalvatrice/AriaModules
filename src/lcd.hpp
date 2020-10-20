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

// This LCD widget displays text, provided by the module or by widgets.
// Its size is currently fixed to 36*10mm - 2 lines of 11 characters.
// Only basic ASCII without accents is suppported.
//
// Modulus Salomonis Regis & Psychopump are the only modules to implement the LCD somewhat
// as intended. Arcane, Darius and QQQQ do things in deprecated (and downright stupid) ways:
// too much logic lives within the module, rather than the widget.
// 
// On Arcane and Darius, the SVG of the LCD is a little bit too small to display
// descenders on the second line of text, so uppercase is mostly used.
// Newer modules have a larger LCD to fit descenders.
//
// It's under heavy rework - I advise against re-using this yet.
// 
// If you're gonna reuse this code despite the warnings, please consider changing my signature
// color scheme to your own. You can recolor the SVG letters in batch with a text editor.

// TODO: Handle the page timer entirely within the widget

namespace Lcd {


// Which elements to show or hide
enum LcdLayouts {
    // Displays text on the first line. Guaranteed a 2nd line will not be used.
    TEXT1_LAYOUT,
    // Displays text on two lines
    TEXT1_AND_TEXT2_LAYOUT,
    // Piano on the first line and text on the second
    PIANO_AND_TEXT2_LAYOUT,
};


// Interface between the module & widgets with the LCD. The module must have one, named lcdStatus.
// It's currently not possible to have more than one LCD as it looks for that name explicitly.
// TODO: Support a decadent amount of LCDs
struct LcdStatus {
    // The first line, not displayed on every layout. 
    std::string text1 = "";

    // The second line, currently displayed on every layout (but not every LCD is visually displayed as having two lines)
    std::string text2 = "";

    // The piano display, displayed on the first line only.
    std::array<bool, 12> pianoDisplay;

    // Whether to redraw the widget.
    bool dirty = false;

    // Whether to draw two lines of text, a piano, etc.
    int layout = TEXT1_AND_TEXT2_LAYOUT;

    // For any info on a timer in the module. This widget has no knowledge what it means.
    // When reset to 0, processLcd() counts up until the notificationTimeout, then sets it to -1.f.
    float lastInteraction = 0.f;

    // How long before going back to the main display.
    float notificationTimeout = 3.f;

    LcdStatus() {
        for (size_t i = 0; i < 12; i++) pianoDisplay[i] = false;
    }

    // Resets the timeout and dirties the framebuffer. Changes layout to TEXT1_LAYOUT upon request.
    void notifyText1(std::string text, bool changeLayout = false) {
        if (changeLayout) layout = TEXT1_LAYOUT;
        text1 = text;
        dirty = true;
        lastInteraction = 0.f;
    }

    // Resets the timeout and dirties the framebuffer. Changes layout to TEXT1_AND_TEXT2_LAYOUT upon request.
    void notifyText2(std::string text, bool changeLayout = false) {
        if (changeLayout) layout = TEXT1_AND_TEXT2_LAYOUT;
        text2 = text;
        dirty = true;
        lastInteraction = 0.f;
    }

    // Call this from the module.
    // Use this to go back to the main page.
    void processLcd(float deltaTime) {
        if (lastInteraction >= 0.f) {
            lastInteraction += deltaTime;
        }
        if (lastInteraction >= notificationTimeout) {
            lastInteraction = -1.f;
        }
    }

};

// The draw widget, concerned only with rendering layouts.
template <class TModule>
struct LcdDrawWidget : LightWidget {
    TModule *module;
    std::array<std::shared_ptr<Svg>, 95> asciiSvg; // 32 to 126, the printable range
    std::array<std::shared_ptr<Svg>, 24> pianoSvg; // 0..11: Unlit, 12..23 = Lit
    std::string text1;
    std::string text2;
    std::string placeholderText1;
    std::string placeholderText2;

    LcdDrawWidget(TModule *_module, std::string _placeholderText1, std::string _placeholderText2) {
        module = _module;
        placeholderText1 = _placeholderText1;
        placeholderText2 = _placeholderText2;
        box.size = mm2px(Vec(36.0, 10.0));
        for (size_t i = 0; i < 12; i++) // Unlit
            pianoSvg[i] = APP->window->loadSvg(asset::plugin(pluginInstance, "res/lcd/piano/u" + std::to_string(i) + ".svg"));
        for (size_t i = 0; i < 12; i++) // Lit
            pianoSvg[i + 12] = APP->window->loadSvg(asset::plugin(pluginInstance, "res/lcd/piano/l" + std::to_string(i) + ".svg"));
        for (size_t i = 0; i < 95; i++)
            asciiSvg[i] = APP->window->loadSvg(asset::plugin(pluginInstance, "res/lcd/Fixed_v01/" + std::to_string(i + 32) + ".svg"));
    }

    // Decides what to draw depending on the layout.
    void draw(const DrawArgs &args) override {

        nvgScale(args.vg, 1.5, 1.5);

        // If there's no module, display instead the placeholder text so it shows up the module browser & online library
        if (!module) { 
            nvgSave(args.vg);
            text1 = placeholderText1;
            text1.append(11, ' '); // Ensure the string is long enough
            for (size_t i = 0; i < 11; i++) {
                char c = text1.at(i);
                svgDraw(args.vg, asciiSvg[ c - 32 ]->handle);
                nvgTranslate(args.vg, 6, 0);
            }
            nvgRestore(args.vg);
            nvgSave(args.vg);
            nvgTranslate(args.vg, 0, 11);
            text2 = placeholderText2;
            text2.append(11, ' '); // Ensure the string is long enough
            for (size_t i = 0; i < 11; i++) {
                char c = text2.at(i);
                svgDraw(args.vg, asciiSvg[ c - 32 ]->handle);
                nvgTranslate(args.vg, 6, 0);
            }
            nvgRestore(args.vg);
            return;
        }

        // Piano display at the top.
        if ( module->lcdStatus.layout == PIANO_AND_TEXT2_LAYOUT ) {
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
        if ( module->lcdStatus.layout == TEXT1_LAYOUT || module->lcdStatus.layout == TEXT1_AND_TEXT2_LAYOUT ) {
            nvgSave(args.vg);
            text1 = module->lcdStatus.text1;
            text1.append(11, ' '); // Ensure the string is long enough
            for (size_t i = 0; i < 11; i++) {
                char c = text1.at(i);
                if (c >= 32 && c <= 126) svgDraw(args.vg, asciiSvg[ c - 32 ]->handle);
                nvgTranslate(args.vg, 6, 0);
            }
            nvgRestore(args.vg);
        }
    
        // 11 character display at the bottom.
        if ( module->lcdStatus.layout == TEXT1_AND_TEXT2_LAYOUT || module->lcdStatus.layout == PIANO_AND_TEXT2_LAYOUT ) {
            nvgSave(args.vg);
            nvgTranslate(args.vg, 0, 11);
            text2 = module->lcdStatus.text2;
            text2.append(11, ' '); // Ensure the string is long enough
            for (size_t i = 0; i < 11; i++) {
                char c = text2.at(i);
                if (c >= 32 && c <= 126) svgDraw(args.vg, asciiSvg[ c - 32 ]->handle);
                nvgTranslate(args.vg, 6, 0);
            }
            nvgRestore(args.vg);
        }
    }

}; // LcdDrawWidget


// The framebuffer holding the Draw widget.
template <typename TModule>
struct LcdFramebufferWidget : FramebufferWidget{
    TModule *module;

    LcdFramebufferWidget(TModule *_module){
        module = _module;
    }

    void step() override{
        if (!module) return;
        if(module->lcdStatus.dirty){
            FramebufferWidget::dirty = true;
            module->lcdStatus.dirty = false;
        }
        FramebufferWidget::step();
    }
};

// The actual LCD widget. Copy it into the module and edit it as needed.
template <typename TModule>
struct LcdWidget : TransparentWidget {
    TModule *module;
    Lcd::LcdFramebufferWidget<TModule> *lfb;
    Lcd::LcdDrawWidget<TModule> *ldw;

    LcdWidget(TModule *_module, std::string placeholderText1 = "", std::string placeholderText2 = ""){
        module = _module;
        lfb = new Lcd::LcdFramebufferWidget<TModule>(module);
        ldw = new Lcd::LcdDrawWidget<TModule>(module, placeholderText1, placeholderText2);
        addChild(lfb);
        lfb->addChild(ldw);
    }

    // Use this to process timeouts and default modes
    void processDefaultMode() {
        if (!module) return;
        if (module->lcdStatus.lastInteraction != -1.f) return;
    }

    void draw(const DrawArgs& args) override {
        processDefaultMode();
        TransparentWidget::draw(args);
    }
};



} // Namespace Lcd
