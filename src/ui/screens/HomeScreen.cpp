#include "HomeScreen.hpp"
#include "../Theme.hpp"
#include <ftxui/dom/elements.hpp>

namespace ymcli::ui::screens {

HomeScreen::HomeScreen() {
    component_ = ftxui::Renderer([this] {
        ftxui::Elements recent_elements;

        if (recent_plays_.empty()) {
            recent_elements.push_back(
                ftxui::text("No recent plays yet. Search for a song to get started (press /).")
                | ftxui::color(Theme::TextTertiary)
            );
        } else {
            for (size_t i = 0; i < recent_plays_.size(); ++i) {
                const auto& play = recent_plays_[i];
                bool is_sel = (static_cast<int>(i) == selected_);
                auto row = ftxui::hbox({
                    ftxui::text(is_sel ? "▸ " : "  ") | ftxui::color(Theme::Accent),
                    ftxui::text(play.title) | ftxui::bold | ftxui::color(is_sel ? Theme::TextPrimary : Theme::TextSecondary) | ftxui::flex,
                    ftxui::text(play.artist) | ftxui::color(Theme::TextTertiary)
                });
                if (is_sel) row = row | ftxui::bgcolor(Theme::Elevated);
                recent_elements.push_back(row);
            }
        }

        return ftxui::vbox({
            ftxui::text("Y O U T U B E   M U S I C") | ftxui::bold | ftxui::color(Theme::Accent) | ftxui::center,
            ftxui::text("Keyboard-driven terminal audio client") | ftxui::color(Theme::TextTertiary) | ftxui::center,
            ftxui::separator() | ftxui::color(Theme::Border),
            
            ftxui::vbox({
                ftxui::text("RECENT PLAYS") | ftxui::bold | ftxui::color(Theme::TextPrimary),
                ftxui::vbox(recent_elements) | ftxui::borderRounded | ftxui::color(Theme::Border)
            }),

            ftxui::separator() | ftxui::color(Theme::Border),

            ftxui::vbox({
                ftxui::text("QUICK NAVIGATION") | ftxui::bold | ftxui::color(Theme::TextPrimary),
                ftxui::hbox({ ftxui::text("  / ") | ftxui::color(Theme::Accent) | ftxui::bold, ftxui::text(" Focus search bar") }),
                ftxui::hbox({ ftxui::text("  space ") | ftxui::color(Theme::Accent) | ftxui::bold, ftxui::text(" Play or pause music") }),
                ftxui::hbox({ ftxui::text("  n / p ") | ftxui::color(Theme::Accent) | ftxui::bold, ftxui::text(" Next / Previous track") }),
                ftxui::hbox({ ftxui::text("  1-7 ") | ftxui::color(Theme::Accent) | ftxui::bold, ftxui::text(" Switch sidebar tabs") })
            }) | ftxui::borderRounded | ftxui::color(Theme::Border)
        }) | ftxui::bgcolor(Theme::Background);
    });

    component_ |= ftxui::CatchEvent([this](ftxui::Event event) {
        if (recent_plays_.empty()) return false;
        if (event == ftxui::Event::Character('j') || event == ftxui::Event::ArrowDown) {
            selected_ = std::min(static_cast<int>(recent_plays_.size() - 1), selected_ + 1);
            return true;
        }
        if (event == ftxui::Event::Character('k') || event == ftxui::Event::ArrowUp) {
            selected_ = std::max(0, selected_ - 1);
            return true;
        }
        return false;
    });
}

ftxui::Component HomeScreen::GetComponent() { return component_; }
void HomeScreen::SetRecentPlays(const std::vector<RecentPlay>& plays) { recent_plays_ = plays; selected_ = 0; }

} // namespace ymcli::ui::screens
