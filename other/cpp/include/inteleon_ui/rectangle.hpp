#pragma once

#include "log.hpp"
#include "types.hpp"
#include "widget.hpp"

#define RECTANGLE_NAME ("native.recatngle")

namespace Inteleon {
	namespace UI {
		class RectangleWidget : public Widget {
			public:

			RectangleWidget(std::string id, Vectors::Vec2D<u32, u32> size)
			: Widget(id, RECTANGLE_NAME, size.x, size.y) {};

			RectangleWidget(std::string id, Vectors::Vec2D<u32, u32> size, std::pixel_rgb color)
			: Widget(id, RECTANGLE_NAME, size.x, size.y), color(color) {};

			void draw(std::display& display, int sx, int sy) const override;

			size_t width() const override;
			size_t height() const override;

			// Fill rectangle or not?
			bool filled = false;
			
			private:

			std::pixel_rgb color {0xff, 0, 0};
		};
	}
}