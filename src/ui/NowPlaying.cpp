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
                ftxui::text("No track playing — press / to search YouTube Music") | ftxui::center | ftxui::color(Theme::TextTertiary)
            }) | ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 3) | ftxui::bgcolor(Theme::SecondaryBg);
        }

        std::string play_icon = state_.is_paused ? " ▶ " : " ❚❚ ";
        std::string track_info = state_.title + " — " + state_.artist;
        std::string time_info = format_time(state_.position) + " / " + format_time(state_.duration);
        float progress = state_.duration > 0 ? static_cast<float>(state_.position / state_.duration) : 0.0f;

        auto row1 = ftxui::hbox({
            ftxui::text(play_icon) | ftxui::bold | ftxui::color(state_.is_paused ? Theme::TextSecondary : Theme::PlayingIndicator),
            ftxui::text(track_info) | ftxui::bold | ftxui::color(Theme::TextPrimary) | ftxui::flex,
            ftxui::text(time_info) | ftxui::color(Theme::TextSecondary)
        });

        auto progress_gauge = ftxui::gauge(progress) | ftxui::color(Theme::Accent) | ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 1);

        std::string mode_str = "";
        if (state_.is_shuffled) mode_str += "🔀 ";
        if (state_.repeat == RepeatMode::All) mode_str += "🔁 ";
        else if (state_.repeat == RepeatMode::One) mode_str += "🔂 ";

        std::string vol_str = "Vol " + std::to_string(static_cast<int>(state_.volume)) + "%";

        auto row2 = ftxui::hbox({
            ftxui::text(state_.album.empty() ? "YouTube Music" : state_.album) | ftxui::color(Theme::TextTertiary) | ftxui::flex,
            ftxui::text("space: pause | n: next | p: prev | +/-: volume") | ftxui::color(Theme::TextTertiary) | ftxui::center | ftxui::flex,
            ftxui::text(mode_str) | ftxui::color(Theme::Accent),
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
