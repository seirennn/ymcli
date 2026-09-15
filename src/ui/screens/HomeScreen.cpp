#include "HomeScreen.hpp"
#include "../Theme.hpp"
#include <ftxui/dom/elements.hpp>

namespace ymcli::ui::screens {

HomeScreen::HomeScreen(PlayTracksCallback play_cb, EnqueueCallback enqueue_cb)
    : play_cb_(std::move(play_cb)), enqueue_cb_(std::move(enqueue_cb))
{
    component_ = ftxui::Renderer([this](bool focused) {
        ftxui::Elements recent_elements;

        if (recent_tracks_.empty()) {
            recent_elements.push_back(
                ftxui::text("  No recent tracks. Press / to search and play.")
                | ftxui::color(Theme::TextTertiary)
            );
        } else {
            for (size_t i = 0; i < recent_tracks_.size(); ++i) {
                const auto& track = recent_tracks_[i];
                bool is_sel = (static_cast<int>(i) == selected_);
                auto row = ftxui::hbox({
                    ftxui::text(is_sel ? "> " : "  ") | ftxui::bold | ftxui::color(is_sel ? (focused ? Theme::Accent : Theme::TextSecondary) : Theme::TextTertiary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 3),
                    ftxui::text(track.title) | ftxui::bold | ftxui::color(is_sel ? Theme::TextPrimary : Theme::TextSecondary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 40),
                    ftxui::text(track.artist) | ftxui::color(is_sel ? (focused ? Theme::Accent : Theme::TextSecondary) : Theme::TextTertiary) | ftxui::flex
                });
                if (is_sel) row = row | ftxui::bgcolor(focused ? Theme::Elevated : Theme::Surface);
                recent_elements.push_back(row);
            }
        }

        auto entry = [](const std::string& cmd, const std::string& flag, const std::string& desc) {
            return ftxui::hbox({
                ftxui::text("  $ ") | ftxui::color(Theme::CmdPrefix),
                ftxui::text(cmd) | ftxui::bold | ftxui::color(Theme::Accent) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 12),
                ftxui::text(flag) | ftxui::color(Theme::KeywordBlue) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 16),
                ftxui::text(desc) | ftxui::color(Theme::TextSecondary)
            });
        };

        return ftxui::vbox({
            ftxui::hbox({
                Theme::cmd_header("ymcli", "--dashboard", "keyboard-driven music client"),
                ftxui::filler(),
                ftxui::text("● ready") | ftxui::color(Theme::Success)
            }),
            ftxui::separator() | ftxui::color(Theme::BorderLight),
            
            ftxui::vbox({
                ftxui::hbox({
                    Theme::cmd_header("tracks", "--recent"),
                    ftxui::filler(),
                    ftxui::text("[j/k] select  [Enter] play  [a] queue") | ftxui::color(Theme::TextTertiary)
                }),
                ftxui::separator() | ftxui::color(Theme::BorderLight),
                ftxui::vbox(std::move(recent_elements)) | ftxui::yframe | ftxui::size(ftxui::HEIGHT, ftxui::LESS_THAN, 10)
            }),

            ftxui::separator() | ftxui::color(Theme::BorderLight),

            ftxui::vbox({
                Theme::cmd_header("shortcuts", "--quickref"),
                ftxui::separator() | ftxui::color(Theme::BorderLight),
                entry("search", "/", "Search YouTube Music"),
                entry("pane", "Shift+←/→", "Switch sidebar ↔ content pane"),
                entry("visualizer", "Shift+F / F", "Fullscreen ambient visualizer"),
                entry("playback", "Space", "Play / pause stream"),
                entry("track", "n / p", "Next / previous track"),
                entry("seek", "< / >", "Seek 10 seconds"),
                entry("navigate", "1 - 7", "Switch view screens"),
                entry("cheatsheet", "?", "Open complete shortcuts modal"),
                entry("exit", "q", "Quit client")
            })
        }) | ftxui::bgcolor(Theme::Background);
    });

    component_ |= ftxui::CatchEvent([this](ftxui::Event event) {
        if (recent_tracks_.empty()) return false;
        if (event == ftxui::Event::Character('j') || event == ftxui::Event::ArrowDown) {
            selected_ = std::min(static_cast<int>(recent_tracks_.size() - 1), selected_ + 1);
            return true;
        }
        if (event == ftxui::Event::Character('k') || event == ftxui::Event::ArrowUp) {
            selected_ = std::max(0, selected_ - 1);
            return true;
        }
        if (event == ftxui::Event::Return) {
            if (play_cb_ && selected_ >= 0 && selected_ < static_cast<int>(recent_tracks_.size())) {
                play_cb_(recent_tracks_, selected_);
            }
            return true;
        }
        if (event == ftxui::Event::Character('a')) {
            if (enqueue_cb_ && selected_ >= 0 && selected_ < static_cast<int>(recent_tracks_.size())) {
                enqueue_cb_(recent_tracks_[selected_]);
            }
            return true;
        }
        return false;
    });
}

ftxui::Component HomeScreen::GetComponent() { return component_; }
void HomeScreen::SetRecentTracks(const std::vector<Track>& tracks) {
    recent_tracks_ = tracks;
    if (selected_ >= static_cast<int>(recent_tracks_.size())) {
        selected_ = std::max(0, static_cast<int>(recent_tracks_.size()) - 1);
    }
}

} // namespace ymcli::ui::screens
