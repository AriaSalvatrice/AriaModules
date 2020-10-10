#pragma once
using namespace rack;
extern Plugin* pluginInstance;

namespace XXX {


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

    void draw(const DrawArgs& args) override {
        if (svg && svg->handle && !hidden) {
            svgDraw(args.vg, svg->handle);
        }
    }
};

// This is a SvgSwitch with special support for LightsOff: every frame past the first
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


struct ReducedButton : LitSvgSwitch {
    ReducedButton() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-700-off.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-700-on.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/pushbutton-700-pink.svg")));
    }
};



} // namespace XXX
