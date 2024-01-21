#include "inteleon_ui/widget.hpp"
#include "inteleon_ui/vectors.hpp"

Inteleon::UI::Widget::Widget(std::string id, std::string name, int width, int height)
			: size(width, height), id(id), name(name) {};

Inteleon::UI::Widget::~Widget() {};