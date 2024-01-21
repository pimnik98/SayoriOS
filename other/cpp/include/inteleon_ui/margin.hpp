#pragma once

#include "../vector.hpp"
#include "types.hpp"
#include "widget.hpp"

#define MARGIN_NAME ("native.modifiers.margin")

namespace Inteleon {
	namespace UI {
		class Margin : public Widget {
			public:

			Margin(std::string id, Widget* widget, Vectors::Vec4D<size_t, size_t, size_t, size_t> pad)
			: Widget(id, MARGIN_NAME, -1, -1), pad(pad), widget(widget) {};

            void draw(std::display& display, int sx, int sy) const override;
            
			size_t width() const override;
			size_t height() const override;

			Vectors::Vec4D<size_t, size_t, size_t, size_t> pad;
			Widget* widget = nullptr;
		};
	}
}
