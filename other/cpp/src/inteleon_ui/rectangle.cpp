#include "log.hpp"
#include "inteleon_ui/rectangle.hpp"

void Inteleon::UI::RectangleWidget::draw(std::display& display, int sx, int sy) const {
    if(filled) {
        display.draw_filled_rect(sx, sy, size.x, size.y, color.r, color.g, color.b);
    } else {
        display.draw_rectangle(sx, sy, size.x, size.y, color.r, color.g, color.b);
    }
}

size_t Inteleon::UI::RectangleWidget::width() const {
    return size.x;
}

size_t Inteleon::UI::RectangleWidget::height() const {
    return size.y;
}