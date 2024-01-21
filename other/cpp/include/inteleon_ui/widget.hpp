#pragma once

#include "types.hpp"
#include "vectors.hpp"
#include "string.hpp"
#include "display.hpp"

namespace Inteleon {
	namespace UI {
		class Widget {
			public:

			Widget(std::string id, std::string name, int width, int height);
			~Widget();

			// Draw
			virtual void draw(std::display& display, int sx, int sy) const = 0;
			
			// Calculate width
			virtual size_t width() const = 0;
			
			// Calculate height
			virtual size_t height() const = 0;

			Vectors::Vec2D<u32, u32> size;

			private:
			std::string id;
			std::string name;
		};
	}
}
