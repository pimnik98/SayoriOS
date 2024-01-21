#pragma once

#include "../vector.hpp"
#include "types.hpp"
#include "widget.hpp"

#define BORDER_NAME ("native.debug.border")

namespace Inteleon {
	namespace UI {
		class Border : public Widget {
			public:

			Border(const std::string& id, Widget* widget, size_t thickness)
			: Widget(id, BORDER_NAME, -1, -1), widget(widget) {};

            void draw(std::display& display, int sx, int sy) const override;
            
			size_t width() const override;
			size_t height() const override;

			Widget* widget = nullptr;

			std::pixel_rgb color {0xff, 0xff, 0xff};
		};
	}
}
