#include "Sidebar.hpp"
#include "Theme.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>

namespace ymcli::ui {

Sidebar::Sidebar(int* active_screen)
    : selected_ptr_(active_screen ? active_screen : &dummy_selected_)
{
    ftxui::MenuOption option;
    option.entries_option.transform = [](const ftxui::EntryState& state) {
        if (state.active) {
            return ftxui::hbox({
                ftxui::text("› ") | ftxui::bold | ftxui::color(Theme::Accent),
                ftxui::text(state.label) | ftxui::color(Theme::TextPrimary) | ftxui::bold
            }) | ftxui::bgcolor(Theme::Elevated);
        }
        return ftxui::hbox({
            ftxui::text("  ") | ftxui::color(Theme::TextTertiary),
            ftxui::text(state.label) | ftxui::color(Theme::TextSecondary)
        });
    };

    menu_ = ftxui::Menu(&items_, selected_ptr_, option);

    component_ = ftxui::Renderer(menu_, [this] {
        auto title = ftxui::vbox({
            ftxui::text(""),
            ftxui::text("ymcli") | ftxui::bold | ftxui::color(Theme::Accent) | ftxui::center,
            ftxui::text("v0.1.0") | ftxui::color(Theme::TextTertiary) | ftxui::center,
            ftxui::text("")
        });

        auto footer = ftxui::vbox({
            ftxui::separator() | ftxui::color(Theme::Border),
            ftxui::text("? help") | ftxui::color(Theme::TextTertiary) | ftxui::center,
            ftxui::text("q quit") | ftxui::color(Theme::TextTertiary) | ftxui::center,
            ftxui::text("")
        });

        return ftxui::vbox({
            title,
            ftxui::separator() | ftxui::color(Theme::Border),
            menu_->Render() | ftxui::flex,
            footer
        }) | ftxui::bgcolor(Theme::SecondaryBg);
    });

    component_ |= ftxui::CatchEvent([this](ftxui::Event event) {
        if (event == ftxui::Event::Character('j') || event == ftxui::Event::ArrowDown) {
            *selected_ptr_ = std::min(static_cast<int>(items_.size()) - 1, *selected_ptr_ + 1);
            return true;
        }
        if (event == ftxui::Event::Character('k') || event == ftxui::Event::ArrowUp) {
            *selected_ptr_ = std::max(0, *selected_ptr_ - 1);
            return true;
        }
        if (event.is_character()) {
            char c = event.character()[0];
            if (c >= '1' && c <= '7') {
                *selected_ptr_ = c - '1';
                return true;
            }
        }
        return false;
    });
}

ftxui::Component Sidebar::GetComponent() { return component_; }
int& Sidebar::selected() { return *selected_ptr_; }

} // namespace ymcli::ui
