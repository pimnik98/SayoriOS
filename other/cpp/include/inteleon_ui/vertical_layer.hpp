#pragma once

#include "../vector.hpp"
#include "types.hpp"
#include "widget.hpp"

#define LAYV_NAME ("native.layers.vertical")

namespace Inteleon {
	namespace UI {
		class VerticalLayer : public Widget {
			public:

			VerticalLayer(std::string id)
			: Widget(id, LAYV_NAME, -1, -1) {};

            void draw(std::display& display, int sx, int sy) const override;
            size_t width() const override;
            size_t height() const override;

			void add_widget(Widget* widget);

            private:

            std::vector<Widget*> widgets;
		};
	}
}
