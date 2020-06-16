#pragma once

using namespace rack;
extern Plugin* pluginInstance; // I dunno if needed

// The LCD is only concerned with displaying data,
// 
// TODO: split the Lcd Mode from the visual representation.
// The LCD shouldn't be aware it's in Knob mode, it should
// just be aware it's required to display two lines of text.


namespace Lcd {

enum LcdModes {
	INIT_MODE,
	DEFAULT_MODE,
	SCALE_MODE,
	KNOB_MODE,
	QUANTIZED_MODE,
	CV_MODE,
	MINMAX_MODE,
	ROUTE_MODE,
	SLIDE_MODE
};


// The framebuffer holding the Draw widget.
template <typename T>
struct LcdFramebufferWidget : FramebufferWidget{
	T *module;
	LcdFramebufferWidget(T *m){
		module = m;
	}

	void step() override{
		if (module) { // Required to avoid crashing module browser
			if(module->lcdDirty){
				FramebufferWidget::dirty = true;
				module->lcdDirty = false;
			}
			FramebufferWidget::step();
		}
	}
};


// The draw widget. For now only used with Darius, but I want to re-use it later.
template <class T>
struct LcdDrawWidget : TransparentWidget {
	T *module;
	std::array<std::shared_ptr<Svg>, 95> asciiSvg; // 32 to 126, the printable range
	std::array<std::shared_ptr<Svg>, 24> pianoSvg; // 0..11: Unlit, 12..23 = Lit
	std::string lcdText1;
	std::string lcdText2;

	LcdDrawWidget(T *module) {
		this->module = module;
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

	void draw(const DrawArgs &args) override {
		if (module) {
			nvgScale(args.vg, 1.5, 1.5);
		
			// Piano display at the top
			if (module->lcdMode == Lcd::SCALE_MODE || module->lcdMode == Lcd::QUANTIZED_MODE || module->lcdMode == Lcd::KNOB_MODE ) {
				bool skipPianoDisplay = false;
				if ( module->lcdMode == Lcd::KNOB_MODE && module->params[module->QUANTIZE_TOGGLE_PARAM].getValue() == 0.f)
					skipPianoDisplay = true;
				if (! skipPianoDisplay ) {
					nvgSave(args.vg);
					svgDraw(args.vg, pianoSvg[(module->pianoDisplay[0])  ? 12 :  0 ]->handle);
					nvgTranslate(args.vg, 6, 0);
					svgDraw(args.vg, pianoSvg[(module->pianoDisplay[1])  ? 13 :  1 ]->handle);
					nvgTranslate(args.vg, 5, 0);
					svgDraw(args.vg, pianoSvg[(module->pianoDisplay[2])  ? 14 :  2 ]->handle);
					nvgTranslate(args.vg, 5, 0);
					svgDraw(args.vg, pianoSvg[(module->pianoDisplay[3])  ? 15 :  3 ]->handle);
					nvgTranslate(args.vg, 5, 0);
					svgDraw(args.vg, pianoSvg[(module->pianoDisplay[4])  ? 16 :  4 ]->handle);
					nvgTranslate(args.vg, 7, 0);
					svgDraw(args.vg, pianoSvg[(module->pianoDisplay[5])  ? 17 :  5 ]->handle);
					nvgTranslate(args.vg, 6, 0);
					svgDraw(args.vg, pianoSvg[(module->pianoDisplay[6])  ? 18 :  6 ]->handle);
					nvgTranslate(args.vg, 5, 0);
					svgDraw(args.vg, pianoSvg[(module->pianoDisplay[7])  ? 19 :  7 ]->handle);
					nvgTranslate(args.vg, 5, 0);
					svgDraw(args.vg, pianoSvg[(module->pianoDisplay[8])  ? 20 :  8 ]->handle);
					nvgTranslate(args.vg, 5, 0);
					svgDraw(args.vg, pianoSvg[(module->pianoDisplay[9])  ? 21 :  9 ]->handle);
					nvgTranslate(args.vg, 5, 0);
					svgDraw(args.vg, pianoSvg[(module->pianoDisplay[10]) ? 22 : 10 ]->handle);
					nvgTranslate(args.vg, 5, 0);
					svgDraw(args.vg, pianoSvg[(module->pianoDisplay[11]) ? 23 : 11 ]->handle);
					nvgRestore(args.vg);
				}
			}

			// 11 character display at the top in some modes.
			if (module->lcdMode == Lcd::INIT_MODE || module->lcdMode == Lcd::SLIDE_MODE || module->lcdMode == Lcd::ROUTE_MODE
			 || module->lcdMode == Lcd::MINMAX_MODE    ) {
				nvgSave(args.vg);
				lcdText1 = module->lcdText1;
				lcdText1.append(11, ' '); // Ensure the string is long enough
				for (int i = 0; i < 11; i++) {
					char c = lcdText1.at(i);
					svgDraw(args.vg, asciiSvg[ c - 32 ]->handle);
					nvgTranslate(args.vg, 6, 0);
				}
				nvgRestore(args.vg);
			}
		
			// 11 character display at the bottom in pretty much every mode.
			if (module->lcdMode == Lcd::INIT_MODE       || module->lcdMode == Lcd::SCALE_MODE    || module->lcdMode == Lcd::SLIDE_MODE
			 || module->lcdMode == Lcd::QUANTIZED_MODE  || module->lcdMode == Lcd::CV_MODE       || module->lcdMode == Lcd::ROUTE_MODE
			 || module->lcdMode == Lcd::MINMAX_MODE     || module->lcdMode == Lcd::KNOB_MODE ) {
				nvgSave(args.vg);
				nvgTranslate(args.vg, 0, 11);
				lcdText2 = module->lcdText2;
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

    
} // Lcd