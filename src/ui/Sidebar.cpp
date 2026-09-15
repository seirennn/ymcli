#include "Sidebar.hpp"
#include "Theme.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>

namespace ymcli::ui {

Sidebar::Sidebar() {
    ftxui::MenuOption option;
    option.entries_option.transform = [](const ftxui::EntryState& state) {
        if (state.active) {
            return ftxui::text("▸ " + state.label) | ftxui::color(Theme::TextPrimary) | ftxui::bold;
        }
        return ftxui::text("  " + state.label) | ftxui::color(Theme::TextSecondary);
    };

    menu_ = ftxui::Menu(&items_, &selected_, option);

    component_ = ftxui::Renderer(menu_, [this] {
        auto header = ftxui::text("ymcli") | ftxui::color(Theme::DimAccent) | ftxui::center;
        return ftxui::vbox({
            header,
            ftxui::separator() | ftxui::color(Theme::Border),
            menu_->Render() | ftxui::flex
        }) | ftxui::bgcolor(Theme::SecondaryBg);
    });

    component_ |= ftxui::CatchEvent([this](ftxui::Event event) {
        if (event == ftxui::Event::Character('j')) {
            selected_ = std::min((int)items_.size() - 1, selected_ + 1);
            return true;
        }
        if (event == ftxui::Event::Character('k')) {
            selected_ = std::max(0, selected_ - 1);
            return true;
        }
        if (event.is_character()) {
            char c = event.character()[0];
            if (c >= '1' && c <= '7') {
                selected_ = c - '1';
                return true;
            }
        }
        return false;
    });
}

ftxui::Component Sidebar::GetComponent() { return component_; }
int& Sidebar::selected() { return selected_; }

} // namespace ymcli::ui
