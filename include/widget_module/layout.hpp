#ifndef LAYOUT_HPP
#define LAYOUT_HPP

#include "widget.hpp"

#include <system_module/events/child_event.hpp>
#include <system_module/events/paint_event.hpp>
#include <system_module/object.hpp>

#include <aml/signals/slot.hpp>

namespace twf {

// Base class for Layouts
class Layout : public Widget {
   public:
    Layout();

    // Slots
    sig::Slot<void()> update_layout;

   protected:
    bool paint_event(const Paint_event& event) override;
    virtual void update_geometry() = 0;
    bool too_small_{false};

   private:
    void initialize(); // Object::initialize() exists as well. but not virtual
};

}  // namespace twf
#endif  // LAYOUT_HPP