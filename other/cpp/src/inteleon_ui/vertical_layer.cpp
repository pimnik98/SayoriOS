#include "inteleon_ui/vertical_layer.hpp"

#include "log.hpp"

void Inteleon::UI::VerticalLayer::draw(std::display& display, int sx, int sy) const {
    for(size_t i = 0; i < widgets.get_length(); i++) {
        widgets[i]->draw(display, sx, sy);
        sy += widgets[i]->height();  
    }
}

size_t Inteleon::UI::VerticalLayer::width() const {
    size_t width = 0;

    for(size_t i = 0; i < widgets.get_length(); i++) {
        if(width < widgets[i]->width())
            width = widgets[i]->width();  
    }

    return width;
}

size_t Inteleon::UI::VerticalLayer::height() const {
    size_t height = 0;

    for(size_t i = 0; i < widgets.get_length(); i++) {
        if(height < widgets[i]->height())
            height = widgets[i]->height();
    }

    return height;
}

void Inteleon::UI::VerticalLayer::add_widget(Widget* widget) {
    widgets.push_back(widget);
}
