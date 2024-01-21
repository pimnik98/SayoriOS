#include "inteleon_ui/margin.hpp"

#include "log.hpp"

void Inteleon::UI::Margin::draw(std::display& display, int sx, int sy) const {
    // <LEFT, RIGHT, TOP, BOTTOM>
    widget->draw(display, sx + pad.x, sy + pad.z);
}

size_t Inteleon::UI::Margin::width() const {
    return widget->width() + pad.x + pad.y;
}

size_t Inteleon::UI::Margin::height() const {
    return widget->width() + pad.z + pad.a;
}