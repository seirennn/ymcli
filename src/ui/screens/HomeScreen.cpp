#include "HomeScreen.hpp"
#include "../Theme.hpp"
#include <ftxui/dom/elements.hpp>

namespace ymcli::ui::screens {

HomeScreen::HomeScreen() {
    component_ = ftxui::Renderer([this] {
        ftxui::Elements recent_elements;

        if (recent_plays_.empty()) {
            recent_elements.push_back(
                ftxui::text("  No recent tracks. Press / to search and play.")
                | ftxui::color(Theme::TextTertiary)
            );
        } else {
            for (size_t i = 0; i < recent_plays_.size(); ++i) {
                const auto& play = recent_plays_[i];
                bool is_sel = (static_cast<int>(i) == selected_);
                auto row = ftxui::hbox({
                    ftxui::text(is_sel ? "› " : "  ") | ftxui::color(is_sel ? Theme::Accent : Theme::TextTertiary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 3),
                    ftxui::text(play.title) | ftxui::bold | ftxui::color(is_sel ? Theme::TextPrimary : Theme::TextSecondary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 40),
                    ftxui::text(play.artist) | ftxui::color(Theme::TextTertiary) | ftxui::flex
                });
                if (is_sel) row = row | ftxui::bgcolor(Theme::Elevated);
                recent_elements.push_back(row);
            }
        }

        auto entry = [](const std::string& key, const std::string& desc) {
            return ftxui::hbox({
                ftxui::text("  " + key) | ftxui::bold | ftxui::color(Theme::Accent) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 14),
                ftxui::text(desc) | ftxui::color(Theme::TextSecondary)
            });
        };

        return ftxui::vbox({
            ftxui::vbox({
                ftxui::text("Y M C L I") | ftxui::bold | ftxui::color(Theme::Accent) | ftxui::center,
                ftxui::text("keyboard-driven audio client for YouTube Music") | ftxui::color(Theme::TextTertiary) | ftxui::center
            }),
            ftxui::separator() | ftxui::color(Theme::Border),
            
            ftxui::vbox({
                ftxui::hbox({
                    ftxui::text("RECENT TRACKS") | ftxui::bold | ftxui::color(Theme::TextPrimary),
                    ftxui::filler(),
                    ftxui::text("[j/k] select  [Enter] play") | ftxui::color(Theme::TextTertiary)
                }),
                ftxui::separator() | ftxui::color(Theme::Border),
                ftxui::vbox(std::move(recent_elements)) | ftxui::yframe | ftxui::size(ftxui::HEIGHT, ftxui::LESS_THAN, 10)
            }),

            ftxui::separator() | ftxui::color(Theme::Border),

            ftxui::vbox({
                ftxui::text("QUICK COMMANDS") | ftxui::bold | ftxui::color(Theme::TextPrimary),
                ftxui::separator() | ftxui::color(Theme::Border),
                entry("/", "Search music"),
                entry("Shift+←/→", "Switch sidebar / content pane"),
                entry("Shift+F", "Fullscreen player & visualizer"),
                entry("space", "Play or pause"),
                entry("n / p", "Next / previous track"),
                entry("< / >", "Seek 10s backward / forward"),
                entry("1 - 7", "Switch view tabs"),
                entry("?", "Show all shortcuts cheatsheet"),
                entry("q", "Quit")
            })
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
