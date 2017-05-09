#include <painter_module/glyph.hpp>
#include <painter_module/painter.hpp>
#include <widget_module/border.hpp>

namespace twf {

void Border::set_walls(const Glyph& horizontals, const Glyph& verticals) {
    north_ = horizontals;
    south_ = horizontals;
    east_ = verticals;
    west_ = verticals;
}

void Border::enable_walls() {
    this->enable_north();
    this->enable_south();
    this->enable_west();
    this->enable_east();
}

void Border::disable_walls() {
    this->disable_north();
    this->disable_south();
    this->disable_west();
    this->disable_east();
}

void Border::enable_corners() {
    this->enable_north_west();
    this->enable_north_east();
    this->enable_south_west();
    this->enable_south_east();
}

void Border::disable_corners() {
    this->disable_north_west();
    this->disable_north_east();
    this->disable_south_west();
    this->disable_south_east();
}

}  // namespace twf