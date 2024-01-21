#include "stdio.hpp"
#include "string.hpp"
#include "inteleon_ui/vectors.hpp"
#include "inteleon_ui/rectangle.hpp"
#include "inteleon_ui/vertical_layer.hpp"
#include "inteleon_ui/horizontal_layer.hpp"
#include "inteleon_ui/margin.hpp"

#include "typeinfo.hpp"

using namespace Inteleon::UI;
using namespace Inteleon::Vectors;

void inteleon_test() {
    std::display displ = std::display();

    RectangleWidget my_wgt("my_widget", {100u, 100u});

    VerticalLayer lay("main");
    HorizontalLayer hlay("main_2");

    RectangleWidget rect("rect", {75, 75});
    RectangleWidget rect2("rect2", {40, 40});
    RectangleWidget rect3("rect3", {50, 50}, {0xff, 0xff, 0x00});
    RectangleWidget rect4("rect4", {60, 70});
    RectangleWidget rect5("rect5", {100, 100});
    RectangleWidget rect6("rect6", {20, 20});

    rect3.filled = true;

    Margin margin("margin1", &rect3, {10, 10, 10, 10});

    lay.add_widget(&rect);
    lay.add_widget(&rect2);
    lay.add_widget(&margin);

    hlay.add_widget(&rect4);
    hlay.add_widget(&rect5);
    hlay.add_widget(&rect6);
    
    lay.add_widget(&hlay);

    lay.draw(displ, 0, 0);
}

void iterator_test() {
    std::vector<size_t> numbers;

    numbers.push_back(1);
    numbers.push_back(2);
    numbers.push_back(3);
    numbers.push_back(4);
    numbers.push_back(5);
    
    for(auto i : numbers) {
        std::cout << i << " ";
    }

    std::cout << std::endl;
}

void cpp_test() asm("cpp_test");
void cpp_test() {
    // iterator_test();
    // inteleon_test();
}
