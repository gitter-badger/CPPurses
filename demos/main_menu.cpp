#include "main_menu.hpp"

#include <cppurses/cppurses.hpp>

using namespace cppurses;

namespace demos {

Main_menu::Main_menu() {
    this->initialize();
}

void Main_menu::initialize() {
    hl.height_policy.type(Size_policy::Fixed);
    hl.height_policy.hint(1);
}

}  // namespace demos
