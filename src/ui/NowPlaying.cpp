#include "NowPlaying.hpp"
#include "Theme.hpp"
#include <ftxui/dom/elements.hpp>
#include <iomanip>
#include <sstream>

namespace ymcli::ui {

NowPlaying::NowPlaying(const PlaybackState& state) : state_(state) {}

static std::string format_time(double seconds) {
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
                ftxui::text("No track playing") | ftxui::center | ftxui::color(Theme::TextTertiary) | ftxui::flex
            }) | ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 3) | ftxui::bgcolor(Theme::SecondaryBg);
        }

        std::string play_icon = state_.is_paused ? "▶" : "❚❚";
        std::string track_info = state_.title + " — " + state_.artist;
        std::string time_info = format_time(state_.position) + " / " + format_time(state_.duration);
        float progress = state_.duration > 0 ? (state_.position / state_.duration) : 0.0f;

        auto row1 = ftxui::hbox({
            ftxui::text(play_icon) | ftxui::color(Theme::PlayingIndicator),
            ftxui::text(" " + track_info) | ftxui::color(Theme::TextPrimary) | ftxui::flex,
            ftxui::text(time_info) | ftxui::color(Theme::TextSecondary)
        });

        auto gauge = ftxui::gauge(progress) | ftxui::color(Theme::Accent) | ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 1);

        std::string controls = "Prev [b] | Play/Pause [space] | Next [n]";
        std::string indicators = state_.is_shuffled ? "🔀 " : "";
        if (state_.repeat == RepeatMode::All) indicators += "🔁 ";
        else if (state_.repeat == RepeatMode::One) indicators += "🔂 ";

        float vol_pct = state_.volume / 100.0f;
        auto vol_bar = ftxui::gauge(vol_pct) | ftxui::color(Theme::DimAccent) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 10);

        auto row2 = ftxui::hbox({
            ftxui::text(state_.album) | ftxui::color(Theme::TextTertiary) | ftxui::flex,
            ftxui::text(controls) | ftxui::color(Theme::TextTertiary) | ftxui::center | ftxui::flex,
            ftxui::text(indicators) | ftxui::color(Theme::TextSecondary),
            ftxui::text(" Vol ") | ftxui::color(Theme::TextTertiary),
            vol_bar
        });

        return ftxui::vbox({
            row1,
            gauge,
            row2
        }) | ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 3) | ftxui::bgcolor(Theme::SecondaryBg);
    });
}

} // namespace ymcli::ui
