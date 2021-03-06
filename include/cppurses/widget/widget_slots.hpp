#ifndef WIDGET_WIDGET_SLOTS_HPP
#define WIDGET_WIDGET_SLOTS_HPP
#include <cppurses/painter/color.hpp>
#include <cppurses/system/key.hpp>
#include <cppurses/system/mouse_button.hpp>
#include <cppurses/widget/point.hpp>

#include <signals/slot.hpp>

namespace cppurses {
class Widget;
namespace slot {

sig::Slot<void()> delete_later(Widget& w);
sig::Slot<void()> close(Widget& w);
sig::Slot<void()> hide(Widget& w);
sig::Slot<void()> show(Widget& w);
sig::Slot<void()> repaint(Widget& w);
sig::Slot<void()> update(Widget& w);
sig::Slot<void(Point, Mouse_button)> click(Widget& w);
sig::Slot<void(Mouse_button)> click(Widget& w, Point c);
sig::Slot<void(Point)> click(Widget& w, Mouse_button b);
sig::Slot<void()> click(Widget& w, Point c, Mouse_button b);
sig::Slot<void(Key)> keypress(Widget& w);
sig::Slot<void()> keypress(Widget& w, Key k);
sig::Slot<void(Color)> set_background(Widget& w);
sig::Slot<void()> set_background(Widget& w, Color c);
sig::Slot<void(Color)> set_foreground(Widget& w);
sig::Slot<void()> set_foreground(Widget& w, Color c);
sig::Slot<void()> toggle_cursor(Widget& w);

}  // namespace slot
}  // namespace cppurses

#endif  // WIDGET_WIDGET_SLOTS_HPP
