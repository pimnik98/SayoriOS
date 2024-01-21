#include "inteleon_ui/horizontal_layer.hpp"

#include "log.hpp"

void Inteleon::UI::HorizontalLayer::draw(std::display& display, int sx, int sy) const {
    for(size_t i = 0; i < widgets.get_length(); i++) {
        widgets[i]->draw(display, sx, sy);
        sx += widgets[i]->width();  // Go to right
    }
}

size_t Inteleon::UI::HorizontalLayer::width() const {
    size_t width = 0;

    for(size_t i = 0; i < widgets.get_length(); i++) {
        if(width < widgets[i]->width())
            width = widgets[i]->width();
    }

    return width;
}

size_t Inteleon::UI::HorizontalLayer::height() const {
    size_t height = 0;

    for(size_t i = 0; i < widgets.get_length(); i++) {
        if(height < widgets[i]->height())
            height = widgets[i]->height();
    }

    return height;
}

void Inteleon::UI::HorizontalLayer::add_widget(Widget* widget) {
    widgets.push_back(widget);
}