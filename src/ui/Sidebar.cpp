#include "Sidebar.hpp"
#include "Theme.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>

namespace ymcli::ui {

Sidebar::Sidebar(OnSelectCallback on_select)
    : on_select_(std::move(on_select))
{
    ftxui::MenuOption option;
    option.entries_option.transform = [this](const ftxui::EntryState& state) {
        bool is_focused = menu_ && menu_->Focused();
        if (state.active) {
            if (is_focused) {
                return ftxui::hbox({
                    ftxui::text("> ") | ftxui::bold | ftxui::color(Theme::Accent),
                    ftxui::text("$ ") | ftxui::bold | ftxui::color(Theme::CmdPrefix),
                    ftxui::text(state.label) | ftxui::color(Theme::TextPrimary) | ftxui::bold
                }) | ftxui::bgcolor(Theme::Elevated);
            } else {
                return ftxui::hbox({
                    ftxui::text("  "),
                    ftxui::text("$ ") | ftxui::color(Theme::Accent),
                    ftxui::text(state.label) | ftxui::color(Theme::TextPrimary)
                }) | ftxui::bgcolor(Theme::Surface);
            }
        }
        return ftxui::hbox({
            ftxui::text("  "),
            ftxui::text("$ ") | ftxui::color(Theme::TextTertiary),
            ftxui::text(state.label) | ftxui::color(Theme::TextSecondary)
        });
    };

    menu_ = ftxui::Menu(&items_, &sidebar_index_, option);

    component_ = ftxui::Renderer(menu_, [this] {
        auto title = ftxui::vbox({
            ftxui::hbox({
                Theme::window_dots(),
                ftxui::filler(),
                ftxui::text("● ready") | ftxui::color(Theme::Success)
            }),
            ftxui::separator() | ftxui::color(Theme::BorderLight),
            ftxui::hbox({
                ftxui::text("~/ymcli") | ftxui::bold | ftxui::color(Theme::Accent),
                ftxui::text(" ▊") | ftxui::color(Theme::PrimaryDark),
                ftxui::filler(),
                ftxui::text("v0.1.0") | ftxui::color(Theme::TextTertiary)
            }),
            ftxui::text("")
        });

        auto footer = ftxui::vbox({
            ftxui::separator() | ftxui::color(Theme::BorderLight),
            ftxui::hbox({
                ftxui::text("pane: ") | ftxui::color(Theme::TextTertiary),
                ftxui::text("Shift+←/→") | ftxui::color(Theme::Accent)
            }) | ftxui::center,
            ftxui::hbox({
                ftxui::text("? ") | ftxui::color(Theme::KeywordBlue),
                ftxui::text("help  ") | ftxui::color(Theme::TextTertiary),
                ftxui::text("q ") | ftxui::color(Theme::Danger),
                ftxui::text("quit") | ftxui::color(Theme::TextTertiary)
            }) | ftxui::center,
            ftxui::text("")
        });

        return ftxui::vbox({
            title,
            ftxui::separator() | ftxui::color(Theme::BorderLight),
            menu_->Render() | ftxui::flex,
            footer
        }) | ftxui::bgcolor(Theme::SecondaryBg);
    });

    component_ |= ftxui::CatchEvent([this](ftxui::Event event) {
        if (event == ftxui::Event::Character('j') || event == ftxui::Event::ArrowDown) {
            sidebar_index_ = std::min(static_cast<int>(items_.size()) - 1, sidebar_index_ + 1);
            if (on_select_) on_select_(sidebar_index_);
            return true;
        }
        if (event == ftxui::Event::Character('k') || event == ftxui::Event::ArrowUp) {
            sidebar_index_ = std::max(0, sidebar_index_ - 1);
            if (on_select_) on_select_(sidebar_index_);
            return true;
        }
        if (event == ftxui::Event::Return) {
            if (on_select_) on_select_(sidebar_index_);
            return true;
        }
        if (event.is_character()) {
            char c = event.character()[0];
            if (c >= '1' && c <= '7') {
                sidebar_index_ = c - '1';
                if (on_select_) on_select_(sidebar_index_);
                return true;
            }
        }
        return false;
    });
}

Sidebar::Sidebar(int* legacy_active_screen)
    : Sidebar([legacy_active_screen](int idx) {
        if (legacy_active_screen) *legacy_active_screen = idx;
    })
{
    if (legacy_active_screen && *legacy_active_screen >= 0 && *legacy_active_screen < static_cast<int>(items_.size())) {
        sidebar_index_ = *legacy_active_screen;
    }
}

ftxui::Component Sidebar::GetComponent() { return component_; }

void Sidebar::SetSelectedIndex(int index) {
    if (index >= 0 && index < static_cast<int>(items_.size())) {
        sidebar_index_ = index;
    }
}

} // namespace ymcli::ui
