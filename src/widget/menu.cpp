#include <cppurses/painter/attribute.hpp>
#include <cppurses/painter/glyph_string.hpp>
#include <cppurses/painter/painter.hpp>
#include <cppurses/widget/focus_policy.hpp>
#include <cppurses/widget/widgets/menu.hpp>
#include <cppurses/widget/widgets/push_button.hpp>

#include <signals/signals.hpp>

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <string>
#include <utility>

namespace cppurses {

Menu::Menu_item::Menu_item(Push_button& ref) : button{ref} {}

Menu::Menu(Glyph_string title)
    : title_{this->make_child<Label>(std::move(title))} {
    this->focus_policy = Focus_policy::Strong;
    title_.set_alignment(Alignment::Center);
    title_.brush.add_attributes(Attribute::Bold);
    space1.background_tile = "─";
}

sig::Signal<void()>& Menu::add_item(Glyph_string label) {
    Push_button& button_ref{this->make_child<Push_button>(std::move(label))};
    button_ref.install_event_filter(this);
    items_.emplace_back(button_ref);
    button_ref.height_policy.type(Size_policy::Fixed);
    button_ref.height_policy.hint(1);
    auto& signal_ref{items_.back().selected};
    button_ref.clicked.connect(
        [ this, index = items_.size() - 1 ] { items_[index].selected(); });
    this->update();
    return signal_ref;
}

sig::Signal<void()>& Menu::insert_item(Glyph_string label, std::size_t index) {
    auto button_ptr{std::make_unique<Push_button>(std::move(label))};
    button_ptr->install_event_filter(this);
    button_ptr->height_policy.type(Size_policy::Fixed);
    button_ptr->height_policy.hint(1);
    Push_button& new_button{*button_ptr};
    items_.insert(std::begin(items_) + index, Menu_item{new_button});
    auto& signal_ref{items_[index].selected};
    new_button.clicked.connect([this, index] { items_[index].selected(); });
    this->update();
    return signal_ref;
}

void Menu::remove_item(std::size_t index) {
    if (index >= items_.size()) {
        return;
    }
    this->remove_child(&items_[index].button.get());
    items_.erase(std::begin(items_) + index);
    this->update();
}

void Menu::select_up(std::size_t n) {
    if (selected_index_ > n) {
        selected_index_ -= n;
    } else {
        selected_index_ = 0;
    }
    this->update();
}

void Menu::select_down(std::size_t n) {
    if (items_.empty()) {
        return;
    }
    std::size_t new_index{selected_index_ + n};
    if (new_index >= items_.size()) {
        selected_index_ = items_.size() - 1;
    } else {
        selected_index_ = new_index;
    }
    this->update();
}

void Menu::select_item(std::size_t index) {
    if (items_.empty()) {
        return;
    }
    if (index >= items_.size()) {
        selected_index_ = items_.size() - 1;
    } else {
        selected_index_ = index;
    }
    this->update();
}

std::size_t Menu::size() const {
    return items_.size();
}

bool Menu::paint_event() {
    for (Menu_item& item : items_) {
        item.button.get().brush.remove_attribute(Attribute::Inverse);
    }
    items_[selected_index_].button.get().brush.add_attributes(
        Attribute::Inverse);
    return Vertical_layout::paint_event();
}

bool Menu::key_press_event(Key key, char symbol) {
    if (key == Key::Arrow_down || key == Key::j) {
        this->select_down();
    } else if (key == Key::Arrow_up || key == Key::k) {
        this->select_up();
    } else if (key == Key::Enter) {
        this->call_current_item();
    }
    return true;
}

bool Menu::mouse_press_event(Mouse_button button,
                             Point global,
                             Point local,
                             std::uint8_t device_id) {
    if (button == Mouse_button::ScrollUp) {
        this->select_up();
    } else if (button == Mouse_button::ScrollDown) {
        this->select_down();
    }
    return Widget::mouse_press_event(button, global, local, device_id);
}

bool Menu::mouse_press_event_filter(Event_handler* receiver,
                                    Mouse_button button,
                                    Point global,
                                    Point local,
                                    std::uint8_t device_id) {
    if (button == Mouse_button::ScrollUp) {
        this->select_up();
        return true;
    }
    if (button == Mouse_button::ScrollDown) {
        this->select_down();
        return true;
    }
    return false;
}

void Menu::call_current_item() {
    if (!items_.empty()) {
        items_[selected_index_].selected();
    }
}

namespace slot {

sig::Slot<void(std::size_t)> select_up(Menu& m) {
    sig::Slot<void(std::size_t)> slot{[&m](auto n) { m.select_up(n); }};
    slot.track(m.destroyed);
    return slot;
}

sig::Slot<void()> select_up(Menu& m, std::size_t n) {
    sig::Slot<void()> slot{[&m, n] { m.select_up(n); }};
    slot.track(m.destroyed);
    return slot;
}

sig::Slot<void(std::size_t)> select_down(Menu& m) {
    sig::Slot<void(std::size_t)> slot{[&m](auto n) { m.select_down(n); }};
    slot.track(m.destroyed);
    return slot;
}

sig::Slot<void()> select_down(Menu& m, std::size_t n) {
    sig::Slot<void()> slot{[&m, n] { m.select_down(n); }};
    slot.track(m.destroyed);
    return slot;
}

sig::Slot<void(std::size_t)> select_item(Menu& m) {
    sig::Slot<void(std::size_t)> slot{
        [&m](auto index) { m.select_item(index); }};
    slot.track(m.destroyed);
    return slot;
}

sig::Slot<void()> select_item(Menu& m, std::size_t index) {
    sig::Slot<void()> slot{[&m, index] { m.select_item(index); }};
    slot.track(m.destroyed);
    return slot;
}

}  // namespace slot
}  // namespace cppurses
