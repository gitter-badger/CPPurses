#include <widget_module/layouts/vertical_layout.hpp>

#include <cstddef>
#include <vector>
#include <deque>
#include <tuple>
#include <functional>

#include <system_module/events/move_event.hpp>
#include <system_module/events/resize_event.hpp>
#include <system_module/system.hpp>
#include <widget_module/border.hpp>
#include <widget_module/size_policy.hpp>
#include <painter_module/geometry.hpp>

namespace twf {

std::vector<std::size_t> Vertical_layout::size_widgets() {
    // <Widget*, width, height>
    std::vector<std::tuple<Widget*, std::size_t, std::size_t>> widgets;
    std::size_t total_stretch{0};
    for (Object* c : this->children()) {
        Widget* w{dynamic_cast<Widget*>(c)};
        if (w != nullptr) {
            widgets.push_back(std::make_tuple(w, 0, 0));
            total_stretch += w->geometry().size_policy().vertical_stretch;
        }
    }

    int height_available = this->height();

    // VERTICAL
    // Set Fixed, Minimum and MinimumExpanding to height_hint
    for (auto& tup : widgets) {
        auto policy =
            std::get<0>(tup)->geometry().size_policy().vertical_policy;
        if (policy == Size_policy::Fixed || policy == Size_policy::Minimum ||
            policy == Size_policy::MinimumExpanding) {
            std::get<2>(tup) = std::get<0>(tup)->geometry().height_hint();
            height_available -= std::get<2>(tup);
        }
    }
    if (height_available < 0) {
        too_small_ = true;
        return std::vector<std::size_t>();
    }

    // Set Size_policy::Ignored widgets to their stretch factor height value
    for (auto& tup : widgets) {
        if (std::get<0>(tup)->geometry().size_policy().vertical_policy ==
            Size_policy::Ignored) {
            const float percent =
                std::get<0>(tup)->geometry().size_policy().vertical_stretch /
                static_cast<float>(total_stretch);
            std::size_t height = percent * this->height();
            if (height < std::get<0>(tup)->geometry().min_height()) {
                height = std::get<0>(tup)->geometry().min_height();
            } else if (height > std::get<0>(tup)->geometry().max_height()) {
                height = std::get<0>(tup)->geometry().max_height();
            }
            std::get<2>(tup) = height;
            height_available -= height;
        }
    }

    // Set Maximum, Preferred and Expanding to height_hint
    for (auto& tup : widgets) {
        auto policy =
            std::get<0>(tup)->geometry().size_policy().vertical_policy;
        if (policy == Size_policy::Maximum ||
            policy == Size_policy::Preferred ||
            policy == Size_policy::Expanding) {
            std::get<2>(tup) = std::get<0>(tup)->geometry().height_hint();
            height_available -= std::get<2>(tup);
        }
    }

    // create vector of size references for below if statements
    std::vector<std::tuple<Widget*, std::reference_wrapper<std::size_t>,
                           std::reference_wrapper<std::size_t>>>
        widgets_w_refs;
    for (auto& tup : widgets) {
        widgets_w_refs.push_back(
            std::tie(std::get<0>(tup), std::get<1>(tup), std::get<2>(tup)));
    }
    // If space left, fill in expanding and min_expanding, then if still,
    // preferred and min
    if (height_available > 0) {
        this->distribute_space(widgets_w_refs, height_available);
    }

    // if negative space left, subtract from max and preferred, then if still
    // needed, expanding
    if (height_available < 0) {
        this->collect_space(widgets_w_refs, height_available);
    }

    // HORIZONTAL - repeat the above, but with horizontal properties
    for (auto& tup : widgets) {
        auto policy =
            std::get<0>(tup)->geometry().size_policy().horizontal_policy;
        if (policy == Size_policy::Fixed) {
            std::get<1>(tup) = std::get<0>(tup)->geometry().width_hint();
            if (std::get<1>(tup) > this->width()) {
                too_small_ = true;
                return std::vector<std::size_t>();
            }
        } else if (policy == Size_policy::Ignored ||
                   policy == Size_policy::Preferred ||
                   policy == Size_policy::Expanding) {
            std::get<1>(tup) = this->width();
            if (std::get<1>(tup) > std::get<0>(tup)->geometry().max_width()) {
                std::get<1>(tup) = std::get<0>(tup)->geometry().max_width();
            } else if (std::get<1>(tup) <
                       std::get<0>(tup)->geometry().min_width()) {
                too_small_ = true;
                return std::vector<std::size_t>();
            }
        } else if (policy == Size_policy::Maximum) {
            std::get<1>(tup) = this->width();
            if (std::get<1>(tup) > std::get<0>(tup)->geometry().width_hint()) {
                std::get<1>(tup) = std::get<0>(tup)->geometry().width_hint();
            } else if (std::get<1>(tup) <
                       std::get<0>(tup)->geometry().min_width()) {
                too_small_ = true;
                return std::vector<std::size_t>();
            }
        } else if (policy == Size_policy::Minimum ||
                   policy == Size_policy::MinimumExpanding) {
            std::get<1>(tup) = this->width();
            if (std::get<1>(tup) < std::get<0>(tup)->geometry().width_hint()) {
                std::get<1>(tup) = std::get<0>(tup)->geometry().width_hint();
            }
            if (std::get<1>(tup) > std::get<0>(tup)->geometry().max_width() ||
                std::get<1>(tup) > this->width()) {
                too_small_ = true;
                return std::vector<std::size_t>();
            }
        }
    }

    // Post all Resize_events
    for (auto& tup : widgets) {
        System::post_event(
            std::get<0>(tup),
            std::make_unique<Resize_event>(std::get<1>(tup), std::get<2>(tup)));
    }
    std::vector<std::size_t> heights;
    heights.reserve(widgets.size());
    for (const auto& tup : widgets) {
        heights.push_back(std::get<2>(tup));
    }
    return heights;
}

void Vertical_layout::distribute_space(
    std::vector<std::tuple<Widget*,
                           std::reference_wrapper<std::size_t>,
                           std::reference_wrapper<std::size_t>>> widgets,
    int height_left) {
    // Find total stretch of first group
    std::size_t total_stretch{0};
    for (const auto& tup : widgets) {
        auto policy =
            std::get<0>(tup)->geometry().size_policy().vertical_policy;
        if (policy == Size_policy::Expanding ||
            policy == Size_policy::MinimumExpanding) {
            total_stretch +=
                std::get<0>(tup)->geometry().size_policy().vertical_stretch;
        }
    }

    // Calculate new heights of widgets in new group, if any go over max_height
    // then assign max value and recurse without that widget in vector.
    std::deque<std::size_t> height_additions;
    int index{0};
    auto to_distribute = height_left;
    for (const auto& tup : widgets) {
        auto policy =
            std::get<0>(tup)->geometry().size_policy().vertical_policy;
        if (policy == Size_policy::Expanding ||
            policy == Size_policy::MinimumExpanding) {
            height_additions.push_back(
                (std::get<0>(tup)->geometry().size_policy().vertical_stretch /
                 static_cast<double>(total_stretch)) *
                to_distribute);
            if ((std::get<2>(tup).get() + height_additions.back()) >
                std::get<0>(tup)->geometry().max_height()) {
                height_left -= std::get<0>(tup)->geometry().max_height() -
                               std::get<2>(tup);
                std::get<2>(tup).get() =
                    std::get<0>(tup)->geometry().max_height();
                widgets.erase(std::begin(widgets) + index);
                return distribute_space(widgets, height_left);
            }
        }
        ++index;
    }

    // If it has gotten this far, no widgets were over space, assign calculated
    // values
    for (auto& tup : widgets) {
        auto policy =
            std::get<0>(tup)->geometry().size_policy().vertical_policy;
        if (policy == Size_policy::Expanding ||
            policy == Size_policy::MinimumExpanding) {
            std::get<2>(tup).get() += height_additions.front();
            height_left -= height_additions.front();
            height_additions.pop_front();
        }
    }

    // SECOND GROUP - duplicate of above dependent on Policies to work with.
    // Preferred and Minimum
    if (height_left == 0) {
        return;
    }
    // Find total stretch
    total_stretch = 0;
    for (const auto& tup : widgets) {
        auto policy =
            std::get<0>(tup)->geometry().size_policy().vertical_policy;
        if (policy == Size_policy::Preferred ||
            policy == Size_policy::Minimum || policy == Size_policy::Ignored) {
            total_stretch +=
                std::get<0>(tup)->geometry().size_policy().vertical_stretch;
        }
    }

    // Calculate new heights of widgets in new group, if any go over max_height
    // then assign max value and recurse without that widget in vector.
    height_additions.clear();
    index = 0;
    to_distribute = height_left;
    for (const auto& tup : widgets) {
        auto policy =
            std::get<0>(tup)->geometry().size_policy().vertical_policy;
        if (policy == Size_policy::Preferred ||
            policy == Size_policy::Minimum || policy == Size_policy::Ignored) {
            height_additions.push_back(
                (std::get<0>(tup)->geometry().size_policy().horizontal_stretch /
                 static_cast<double>(total_stretch)) *
                to_distribute);
            if ((std::get<2>(tup).get() + height_additions.back()) >
                std::get<0>(tup)->geometry().max_height()) {
                height_left -= std::get<0>(tup)->geometry().max_height() -
                               std::get<2>(tup);
                std::get<2>(tup).get() =
                    std::get<0>(tup)->geometry().max_height();
                widgets.erase(std::begin(widgets) + index);
                return distribute_space(widgets, height_left);
            }
        }
        ++index;
    }

    // If it has gotten this far, no widgets were over space, assign calculated
    // values
    for (auto& tup : widgets) {
        auto policy =
            std::get<0>(tup)->geometry().size_policy().vertical_policy;
        if (policy == Size_policy::Preferred ||
            policy == Size_policy::Minimum || policy == Size_policy::Ignored) {
            std::get<2>(tup).get() += height_additions.front();
            height_left -= height_additions.front();
            height_additions.pop_front();
        }
    }
    if (height_left == 0) {
        return;
    }
    // Rounding error extra
    // First Group
    auto height_check{height_left};
    do {
        height_check = height_left;
        for (auto& tup : widgets) {
            auto policy =
                std::get<0>(tup)->geometry().size_policy().vertical_policy;
            if ((policy == Size_policy::Expanding ||
                 policy == Size_policy::MinimumExpanding) &&
                height_left > 0) {
                if (std::get<2>(tup).get() + 1 <=
                    std::get<0>(tup)->geometry().max_height()) {
                    std::get<2>(tup).get() += 1;
                    height_left -= 1;
                }
            }
        }
    } while (height_check != height_left);

    // Second Group
    do {
        height_check = height_left;
        for (auto& tup : widgets) {
            auto policy =
                std::get<0>(tup)->geometry().size_policy().vertical_policy;
            if ((policy == Size_policy::Preferred ||
                 policy == Size_policy::Minimum ||
                 policy == Size_policy::Ignored) &&
                height_left > 0) {
                if (std::get<2>(tup).get() + 1 <=
                    std::get<0>(tup)->geometry().max_height()) {
                    std::get<2>(tup).get() += 1;
                    height_left -= 1;
                }
            }
        }
    } while (height_check != height_left);
}

void Vertical_layout::collect_space(
    std::vector<std::tuple<Widget*,
                           std::reference_wrapper<std::size_t>,
                           std::reference_wrapper<std::size_t>>> widgets,
    int height_left) {
    if (height_left == 0) {
        return;
    }
    // Find total stretch of first group
    std::size_t total_stretch{0};
    for (const auto& tup : widgets) {
        auto policy =
            std::get<0>(tup)->geometry().size_policy().vertical_policy;
        if (policy == Size_policy::Maximum ||
            policy == Size_policy::Preferred ||
            policy == Size_policy::Ignored) {
            total_stretch +=
                std::get<0>(tup)->geometry().size_policy().vertical_stretch;
        }
    }

    // Find total of inverse of percentages
    double total_inverse{0};
    for (const auto& tup : widgets) {
        auto policy =
            std::get<0>(tup)->geometry().size_policy().vertical_policy;
        if (policy == Size_policy::Maximum ||
            policy == Size_policy::Preferred ||
            policy == Size_policy::Ignored) {
            total_inverse +=
                1 /
                (std::get<0>(tup)->geometry().size_policy().vertical_stretch /
                 static_cast<double>(total_stretch));
        }
    }

    // Calculate new heights of widgets in new group, if any go under min_height
    // then assign min value and recurse without that widget in vector.
    std::deque<std::size_t> height_deductions;
    int index{0};
    auto to_collect = height_left;
    for (const auto& tup : widgets) {
        auto policy =
            std::get<0>(tup)->geometry().size_policy().vertical_policy;
        if (policy == Size_policy::Maximum ||
            policy == Size_policy::Preferred ||
            policy == Size_policy::Ignored) {
            height_deductions.push_back(
                ((1 / (std::get<0>(tup)
                           ->geometry()
                           .size_policy()
                           .horizontal_stretch /
                       static_cast<double>(total_stretch))) /
                 static_cast<double>(total_inverse)) *
                (to_collect * -1));
            if ((std::get<2>(tup).get() - height_deductions.back()) <
                std::get<0>(tup)->geometry().min_height()) {
                height_left += std::get<2>(tup).get() -
                               std::get<0>(tup)->geometry().min_height();
                std::get<2>(tup).get() =
                    std::get<0>(tup)->geometry().min_height();
                widgets.erase(std::begin(widgets) + index);
                return collect_space(widgets, height_left);
            }
        }
        ++index;
    }

    // If it has gotten this far, no widgets were over space, assign calculated
    // values
    for (auto& tup : widgets) {
        auto policy =
            std::get<0>(tup)->geometry().size_policy().vertical_policy;
        if (policy == Size_policy::Maximum ||
            policy == Size_policy::Preferred ||
            policy == Size_policy::Ignored) {
            std::get<2>(tup).get() -= height_deductions.front();
            height_left += height_deductions.front();
            height_deductions.pop_front();
        }
    }

    // SECOND GROUP - duplicate of above dependent on Policies to work with.
    if (height_left == 0) {
        return;
    }
    // Find total stretch
    total_stretch = 0;
    for (const auto& tup : widgets) {
        auto policy = std::get<0>(tup)->geometry().size_policy().vertical_policy;
        if (policy == Size_policy::Expanding) {
            total_stretch +=
                std::get<0>(tup)->geometry().size_policy().vertical_stretch;
        }
    }

    // Find total of inverse of percentages
    total_inverse = 0;
    for (const auto& tup : widgets) {
        auto policy =
            std::get<0>(tup)->geometry().size_policy().vertical_policy;
        if (policy == Size_policy::Expanding) {
            total_inverse +=
                1 /
                (std::get<0>(tup)->geometry().size_policy().vertical_stretch /
                 static_cast<double>(total_stretch));
        }
    }

    // Calculate new heights of widgets in new group, if any go over max_height
    // then assign max value and recurse without that widget in vector.
    height_deductions.clear();
    index = 0;
    to_collect = height_left;
    for (const auto& tup : widgets) {
        auto policy =
            std::get<0>(tup)->geometry().size_policy().vertical_policy;
        if (policy == Size_policy::Expanding) {
            height_deductions.push_back(
                ((1 / (std::get<0>(tup)
                           ->geometry()
                           .size_policy()
                           .horizontal_stretch /
                       static_cast<double>(total_stretch))) /
                 static_cast<double>(total_inverse)) *
                (to_collect * -1));
            if ((std::get<2>(tup).get() - height_deductions.back()) <
                std::get<0>(tup)->geometry().min_height()) {
                height_left += std::get<2>(tup).get() -
                               std::get<0>(tup)->geometry().min_height();
                std::get<2>(tup).get() =
                    std::get<0>(tup)->geometry().min_height();
                widgets.erase(std::begin(widgets) + index);
                return collect_space(widgets, height_left);
            }
        }
        ++index;
    }

    // If it has gotten this far, no widgets were over space, assign calculated
    // values
    for (auto& tup : widgets) {
        auto policy =
            std::get<0>(tup)->geometry().size_policy().vertical_policy;
        if (policy == Size_policy::Expanding) {
            std::get<2>(tup).get() -= height_deductions.front();
            height_left += height_deductions.front();
            height_deductions.pop_front();
        }
    }
    // Change this to distribute the space, it might not be too small
    if (height_left != 0) {
        too_small_ = true;
        return;
    }
}

void Vertical_layout::position_widgets(
    const std::vector<std::size_t>& heights) {
    std::vector<Widget*> widgets;
    for (Object* c : this->children()) {
        Widget* w{dynamic_cast<Widget*>(c)};
        if (w != nullptr) {
            widgets.push_back(w);
        }
    }
    std::size_t x_pos{0};
    std::size_t y_pos{0};
    std::size_t index{0};
    if (widgets.size() != heights.size()) {
        return;
    }
    if ((this->border().west_enabled() || this->border().north_west_enabled() ||
         this->border().south_west_enabled()) &&
        this->border().enabled()) {
        ++x_pos;
    }
    if ((this->border().north_enabled() ||
         this->border().north_west_enabled() ||
         this->border().north_east_enabled()) &&
        this->border().enabled()) {
        ++y_pos;
    }
    for (auto& widg : widgets) {
        System::post_event(widg, std::make_unique<Move_event>(
                                     this->x() + x_pos, this->y() + y_pos));
        y_pos += heights.at(index);
        ++index;
    }
}

void Vertical_layout::update_geometry() {
    auto heights = this->size_widgets();
    this->position_widgets(heights);
}

// void Vertical_layout::update_geometry() {
//     std::size_t border_space{0};
//     std::size_t total_stretch{0};
//     for (const Object* c : this->children()) {
//         const Widget* cw{dynamic_cast<const Widget*>(c)};
//         if (cw != nullptr) {
//             // Border space
//             if ((cw->border().north_enabled() ||
//                  cw->border().north_west_enabled() ||
//                  cw->border().north_east_enabled()) &&
//                 cw->border().enabled()) {
//                 ++border_space;
//             }
//             if ((cw->border().south_enabled() ||
//                  cw->border().south_west_enabled() ||
//                  cw->border().south_east_enabled()) &&
//                 cw->border().enabled()) {
//                 ++border_space;
//             }
//             // Stretch factor
//             total_stretch += cw->size_policy().vertical_stretch;
//         }
//     }
//     std::size_t widg_space{0};
//     if (this->geometry().height() < border_space) {
//         widg_space = 0;
//     } else {
//         widg_space = this->geometry().height() - border_space;
//     }

//     std::size_t y_pos{0};
//     for (Object* c : this->children()) {
//         Widget* cw{dynamic_cast<Widget*>(c)};
//         if (cw != nullptr) {
//             // Position
//             if ((cw->border().north_enabled() ||
//                  cw->border().north_west_enabled() ||
//                  cw->border().north_east_enabled()) &&
//                 cw->border().enabled()) {
//                 ++y_pos;
//             }
//             std::size_t x_pos{0};
//             if ((cw->border().west_enabled() ||
//                  cw->border().north_west_enabled() ||
//                  cw->border().south_west_enabled()) &&
//                 cw->border().enabled()) {
//                 ++x_pos;
//             }
//             // Size
//             if (total_stretch == 0) {
//                 total_stretch = this->children().size();
//             }
//             std::size_t height =
//                 widg_space *
//                 (cw->size_policy().vertical_stretch / double(total_stretch));
//             std::size_t width{this->geometry().width()};
//             if ((cw->border().west_enabled() ||
//                  cw->border().north_west_enabled() ||
//                  cw->border().south_west_enabled()) &&
//                 cw->border().enabled() && width != 0) {
//                 --width;
//             }
//             if ((cw->border().east_enabled() ||
//                  cw->border().north_east_enabled() ||
//                  cw->border().south_east_enabled()) &&
//                 cw->border().enabled() && width != 0) {
//                 --width;
//             }
//             System::post_event(cw,
//                                std::make_unique<Resize_event>(width,
//                                height));
//             System::post_event(cw, std::make_unique<Move_event>(
//                                        this->x() + x_pos, this->y() +
//                                        y_pos));
//             y_pos += height;
//             if ((cw->border().south_enabled() ||
//                  cw->border().south_west_enabled() ||
//                  cw->border().south_east_enabled()) &&
//                 cw->border().enabled()) {
//                 ++y_pos;
//             }
//             // if last widget, extend to rest of layout
//             if (c == *(--std::end(this->children())) &&
//                 this->geometry().height() > y_pos) {
//                 System::post_event(
//                     cw, std::make_unique<Resize_event>(
//                             width, height + this->geometry().height() -
//                             y_pos));
//             }
//         }
//     }
// }

}  // namespace twf