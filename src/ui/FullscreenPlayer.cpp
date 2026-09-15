#include "FullscreenPlayer.hpp"
#include "Theme.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/event.hpp>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace ymcli::ui {

FullscreenPlayer::FullscreenPlayer(const PlaybackState& state)
    : state_(state)
{
}

void FullscreenPlayer::Tick() {
    if (state_.has_track && !state_.is_paused) {
        // Slow, continuous ambient phase progression (approx 0.04 per tick at 16 FPS)
        anim_phase_ += 0.04;
        if (anim_phase_ > 100000.0) anim_phase_ = 0.0;
    }
}

static std::string format_time(double seconds) {
    if (seconds < 0) seconds = 0;
    int m = static_cast<int>(seconds) / 60;
    int s = static_cast<int>(seconds) % 60;
    std::ostringstream oss;
    oss << m << ":" << std::setfill('0') << std::setw(2) << s;
    return oss.str();
}

ftxui::Element FullscreenPlayer::RenderVisualizer(int num_bars, int max_height) {
    static const std::vector<std::string> blocks = {
        " ", " ", "▂", "▃", "▄", "▅", "▆", "▇", "█"
    };

    ftxui::Elements columns;
    for (int col = 0; col < num_bars; ++col) {
        // Normalized index 0.0 .. 1.0
        double norm_idx = static_cast<double>(col) / std::max(1, num_bars - 1);

        // Acoustic spectral envelope (bell-shaped resonance emphasizing low-mids)
        double envelope = 0.35 + 0.65 * std::sin(norm_idx * 3.14159265);
        if (norm_idx < 0.15) {
            envelope = 0.4 + 0.6 * (norm_idx / 0.15); // gentle sub-bass taper
        }

        double val = 0.08;
        if (state_.has_track) {
            // Harmonic wave synthesis: calm, organic fluctuations without jarring spikes
            double h1 = std::sin(anim_phase_ * 0.9 + col * 0.22);
            double h2 = std::cos(anim_phase_ * 0.55 - col * 0.14 + 1.1);
            double h3 = std::sin(anim_phase_ * 0.3 + col * 0.45 + 2.3);

            double dynamic_mod = 0.52 + 0.26 * h1 + 0.14 * h2 + 0.08 * h3;
            val = envelope * dynamic_mod;
            if (val < 0.06) val = 0.06;
            if (val > 0.95) val = 0.95;
        }

        // Generate vertical column of elements (from top row to bottom row)
        double total_level = val * max_height;
        ftxui::Elements col_cells;

        for (int r = max_height - 1; r >= 0; --r) {
            double cell_fill = total_level - r;
            std::string glyph = " ";
            ftxui::Color cell_color = Theme::SecondaryBg;

            if (cell_fill >= 1.0) {
                glyph = "█";
            } else if (cell_fill > 0.0) {
                int block_idx = static_cast<int>(cell_fill * 8.0);
                block_idx = std::clamp(block_idx, 1, 8);
                glyph = blocks[block_idx];
            }

            // Subdued vertical color gradient: warm slate base to warm terracotta peak
            if (r >= 7) {
                cell_color = Theme::Accent; // Peak: #d99178
            } else if (r >= 4) {
                cell_color = Theme::DimAccent; // Mid: #b56850
            } else {
                cell_color = ftxui::Color::RGB(55, 45, 42); // Base: subdued warm charcoal
            }

            col_cells.push_back(ftxui::text(glyph) | ftxui::color(cell_color));
        }

        columns.push_back(ftxui::vbox(std::move(col_cells)));
    }

    return ftxui::hbox(std::move(columns)) | ftxui::center;
}

ftxui::Component FullscreenPlayer::GetComponent() {
    return ftxui::Renderer([this]() -> ftxui::Element {
        // 1. Top Header Bar with macOS Window Dots
        auto top_bar = ftxui::hbox({
            Theme::window_dots(),
            ftxui::text("  ~/ymcli/visualizer") | ftxui::bold | ftxui::color(Theme::Accent),
            ftxui::text(" ▊") | ftxui::color(Theme::PrimaryDark),
            ftxui::text("  ") | ftxui::color(Theme::BorderLight),
            ftxui::text(state_.has_track && !state_.is_paused ? "● streaming" : "● paused") | ftxui::color(state_.has_track && !state_.is_paused ? Theme::Success : Theme::Accent),
            ftxui::filler(),
            ftxui::text("[Shift+F / Esc] Exit  ") | ftxui::color(Theme::TextTertiary)
        }) | ftxui::bgcolor(Theme::SecondaryBg);

        // 2. Center Visualizer Area
        int num_bars = 48;
        int vis_height = 10;
        auto visualizer_elem = RenderVisualizer(num_bars, vis_height);

        std::string title_str = state_.has_track ? (state_.title.empty() ? "Unknown Title" : state_.title) : "No track playing";
        std::string artist_str = state_.has_track ? (state_.artist.empty() ? "Unknown Artist" : state_.artist) : "Search music with /";
        std::string album_str = state_.has_track ? (state_.album.empty() ? "YouTube Music" : state_.album) : "";

        std::string status_badge = state_.has_track ? (state_.is_paused ? "❚❚ paused" : "● playing") : "● idle";
        auto status_color = state_.has_track ? (state_.is_paused ? ftxui::color(Theme::Accent) : ftxui::color(Theme::Success)) : ftxui::color(Theme::TextTertiary);

        ftxui::Elements cmd_line;
        cmd_line.push_back(ftxui::text("$ ") | ftxui::bold | ftxui::color(Theme::CmdPrefix));
        cmd_line.push_back(ftxui::text("now-playing: ") | ftxui::bold | ftxui::color(Theme::KeywordBlue));
        cmd_line.push_back(ftxui::text("\"" + title_str + "\"") | ftxui::bold | ftxui::color(Theme::TextPrimary));

        ftxui::Elements meta_line;
        meta_line.push_back(ftxui::text("artist: ") | ftxui::color(Theme::TextTertiary));
        meta_line.push_back(ftxui::text(artist_str) | ftxui::color(Theme::Accent));
        if (!album_str.empty()) {
            meta_line.push_back(ftxui::text("   album: ") | ftxui::color(Theme::TextTertiary));
            meta_line.push_back(ftxui::text(album_str) | ftxui::color(Theme::TextSecondary));
        }

        auto track_info = ftxui::vbox({
            ftxui::hbox(std::move(cmd_line)) | ftxui::center,
            ftxui::text("") | ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 1),
            ftxui::hbox(std::move(meta_line)) | ftxui::center,
            ftxui::text("") | ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 1),
            ftxui::text(status_badge) | status_color | ftxui::bold | ftxui::center
        });

        auto center_body = ftxui::vbox({
            ftxui::filler(),
            visualizer_elem,
            ftxui::text("") | ftxui::size(ftxui::HEIGHT, ftxui::EQUAL, 2),
            track_info,
            ftxui::filler()
        }) | ftxui::flex;

        // 3. Bottom Playback & Progress Bar
        float progress = 0.0f;
        if (state_.duration > 0.0) {
            progress = static_cast<float>(state_.position / state_.duration);
            progress = std::clamp(progress, 0.0f, 1.0f);
        }

        auto progress_row = ftxui::hbox({
            ftxui::text(" " + format_time(state_.position) + " ") | ftxui::bold | ftxui::color(Theme::KeywordBlue),
            ftxui::gauge(progress) | ftxui::color(Theme::Accent) | ftxui::bgcolor(Theme::CodeBg) | ftxui::flex,
            ftxui::text(" " + format_time(state_.duration) + " ") | ftxui::bold | ftxui::color(Theme::KeywordBlue)
        });

        std::string flags = "";
        if (state_.is_shuffled) flags += "[shuf] ";
        if (state_.repeat == RepeatMode::All) flags += "[repeat] ";
        else if (state_.repeat == RepeatMode::One) flags += "[repeat:1] ";

        std::string vol_str = "vol " + std::to_string(static_cast<int>(state_.volume)) + "%";

        auto status_row = ftxui::hbox({
            ftxui::text(" " + flags) | ftxui::color(Theme::Accent),
            ftxui::text("[mpv::stream] ") | ftxui::color(Theme::KeywordBlue),
            ftxui::filler(),
            ftxui::text(vol_str + " ") | ftxui::color(Theme::TextSecondary)
        });

        auto controls_hint = ftxui::hbox({
            ftxui::text(" [Space] Play/Pause   [n] Next   [p] Prev   [< / >] Seek 10s   [+ / -] Volume   [Shift+F] Exit ")
            | ftxui::color(Theme::TextTertiary) | ftxui::center
        });

        auto bottom_section = ftxui::vbox({
            ftxui::separator() | ftxui::color(Theme::BorderLight),
            progress_row,
            status_row,
            ftxui::separator() | ftxui::color(Theme::BorderLight),
            controls_hint
        }) | ftxui::bgcolor(Theme::SecondaryBg);

        return ftxui::vbox({
            top_bar,
            ftxui::separator() | ftxui::color(Theme::BorderLight),
            center_body,
            bottom_section
        }) | ftxui::bgcolor(Theme::Background);
    });
}

} // namespace ymcli::ui
