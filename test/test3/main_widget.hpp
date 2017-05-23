#ifndef MAIN_WIDGET_HPP
#define MAIN_WIDGET_HPP

#include <twidgets.hpp>
using namespace twf;

class Exit_widget : public Widget {
   public:
    Exit_widget() {
        this->set_focus_policy(Focus_policy::Click);
        this->enable_cursor();
        this->enable_border();
        this->set_brush(
            Brush(background(Color::Green), foreground(Color::Orange)));
    }

    bool key_press_event(const Key_event& ke) override {
        if (ke.key_code() == Key::j) {
            this->close();
        }
        return true;
    }
};

class Color_display : public Widget {
   public:
    Color_display() {
        this->set_focus_policy(Focus_policy::Click);
        for (int i{240}; i < 256; ++i) {
            for (int j{240}; j < 256; ++j) {
                colors_.append("+", background(static_cast<Color>(i)),
                               foreground(static_cast<Color>(j)));
            }
        }
    }

    bool paint_event(const Paint_event& event) override {
        Painter p{this};
        p.put(colors_ + colors_);
        return Widget::paint_event(event);
    }

   private:
    Glyph_string colors_;
};

class Palette_changer : public Text_display {
   public:
    Palette_changer() : Text_display("Click me to enable palette changes") {
        enable_border();
        brush().set_background(Color::Dark_red);
        this->set_focus_policy(Focus_policy::Click);
    }
    bool key_press_event(const Key_event& event) override {
        if (event.text() == "s") {
            System::set_palette(std::make_unique<Standard_palette>());
        } else if (event.text() == "d") {
            System::set_palette(std::make_unique<DawnBringer_palette>());
        }
        return Text_display::key_press_event(event);
    }
};

class Main_widget : public Horizontal_layout {
   public:
    Main_widget() {
        // this->make_child<Exit_widget>();
        this->make_child<Color_display>();
    }
};

#endif  // MAIN_WIDGET_HPP
