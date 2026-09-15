#include "NowPlaying.hpp"
#include "Theme.hpp"
#include <ftxui/dom/elements.hpp>
#include <iomanip>
#include <sstream>

namespace ymcli::ui {

NowPlaying::NowPlaying(const PlaybackState& state) : state_(state) {}

static std::string format_time(double seconds) {
    if (seconds < 0) seconds = 0;
    int m = static_cast<int>(seconds) / 60;
    int s = static_cast<int>(seconds) % 60;
    std::ostringstream oss;
    oss << m << ":" << std::setfill('0') << std::setw(2) << s;
    return oss.str();
}

ftxui::Component NowPlaying::GetComponent() {
    return ftxui::Renderer([this] {
        if (!state_.has_track) {
            return ftxui::vbox({
                ftxui::hbox({
                    ftxui::text(" [IDLE] ") | ftxui::color(Theme::TextTertiary),
                    ftxui::text("Press / to search YouTube Music") | ftxui::color(Theme::TextTertiary) | ftxui::flex,
                    ftxui::text("? help  ") | ftxui::color(Theme::TextTertiary)
                })
            }) | ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 2) | ftxui::bgcolor(Theme::SecondaryBg);
        }

        std::string status_badge = state_.is_paused ? " [PAUSED] " : " [PLAY] ";
        auto status_color = state_.is_paused ? ftxui::color(Theme::TextSecondary) : ftxui::color(Theme::PlayingIndicator);

        std::string time_info = format_time(state_.position) + " / " + format_time(state_.duration);
        float progress = state_.duration > 0 ? static_cast<float>(state_.position / state_.duration) : 0.0f;

        auto row1 = ftxui::hbox({
            ftxui::text(status_badge) | status_color | ftxui::bold,
            ftxui::text(state_.title) | ftxui::bold | ftxui::color(Theme::TextPrimary),
            ftxui::text("  ") | ftxui::color(Theme::TextTertiary),
            ftxui::text(state_.artist) | ftxui::color(Theme::TextSecondary) | ftxui::flex,
            ftxui::text(time_info + " ") | ftxui::color(Theme::TextSecondary)
        });

        auto progress_gauge = ftxui::gauge(progress) | ftxui::color(Theme::Accent) | ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 1);

        std::string flags = "";
        if (state_.is_shuffled) flags += "[SHUF] ";
        if (state_.repeat == RepeatMode::All) flags += "[REPEAT] ";
        else if (state_.repeat == RepeatMode::One) flags += "[REPEAT:1] ";

        std::string vol_str = "vol " + std::to_string(static_cast<int>(state_.volume)) + "% ";

        auto row2 = ftxui::hbox({
            ftxui::text(" " + (state_.album.empty() ? "YouTube Music" : state_.album)) | ftxui::color(Theme::TextTertiary) | ftxui::flex,
            ftxui::text(flags) | ftxui::color(Theme::Accent),
            ftxui::text(vol_str) | ftxui::color(Theme::TextSecondary)
        });

        return ftxui::vbox({
            row1,
            progress_gauge,
            row2
        }) | ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 3) | ftxui::bgcolor(Theme::SecondaryBg);
    });
}

} // namespace ymcli::ui
