#include "log.hpp"
#include "inteleon_ui/border.hpp"

void Inteleon::UI::Border::draw(std::display& display, int sx, int sy) const {
	display.draw_rectangle(sx, sy, size.x, size.y, color.r, color.g, color.b);
	widget->draw(display, sx, sy);
}

size_t Inteleon::UI::Border::width() const {
    return widget->width();
}

size_t Inteleon::UI::Border::height() const {
    return widget->height();
}
