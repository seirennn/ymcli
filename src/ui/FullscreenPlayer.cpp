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
        // Continuous organic phase progression and stereo pan modulation
        anim_phase_ += 0.08;
        pan_phase_ += 0.035;
        if (anim_phase_ > 100000.0) anim_phase_ = 0.0;
        if (pan_phase_ > 100000.0) pan_phase_ = 0.0;
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

    if (bar_levels_.size() != static_cast<size_t>(num_bars)) {
        bar_levels_.assign(num_bars, 0.05);
        peak_levels_.assign(num_bars, 0.05);
    }

    // Volume energy scaling: muted or 0 vol settles calmly, full vol expands
    double vol_scale = 0.05;
    if (state_.has_track && !state_.is_paused) {
        if (state_.is_muted || state_.volume <= 0.0) {
            vol_scale = 0.04;
        } else {
            vol_scale = std::clamp(std::pow(state_.volume / 100.0, 0.8), 0.12, 1.0);
        }
    }

    // Dynamic stereo pan drift across the soundstage
    double pan_bias = std::sin(pan_phase_) * 0.35;

    ftxui::Elements columns;
    for (int col = 0; col < num_bars; ++col) {
        double norm_idx = static_cast<double>(col) / std::max(1, num_bars - 1);

        // Acoustic spectral envelope: low-mid warmth, natural high roll-off
        double envelope = 0.30 + 0.70 * std::sin(norm_idx * 3.14159265);
        if (norm_idx < 0.12) {
            envelope = 0.35 + 0.65 * (norm_idx / 0.12);
        }

        double target = 0.04;
        if (state_.has_track && !state_.is_paused) {
            // Stereo panning modulation (left soundstage vs right soundstage)
            double pan_mod = 1.0 + (norm_idx - 0.5) * 2.0 * pan_bias;
            pan_mod = std::clamp(pan_mod, 0.65, 1.35);

            // Multi-band frequency dynamics
            double band_energy = 0.0;
            if (norm_idx < 0.28) {
                // Sub-bass & Bass: rhythmic beat swell + low rumble
                double beat = std::pow(std::max(0.0, std::sin(state_.position * 3.14159265 * 2.2)), 6.0);
                double sub = std::sin(anim_phase_ * 1.1 + col * 0.35);
                band_energy = 0.48 + 0.38 * beat + 0.22 * sub;
            } else if (norm_idx < 0.72) {
                // Mids & High-Mids: vocal presence and melodic motion
                double m1 = std::sin(anim_phase_ * 0.85 + col * 0.25);
                double m2 = std::cos(anim_phase_ * 0.50 - col * 0.18 + 1.4);
                band_energy = 0.44 + 0.28 * m1 + 0.16 * m2;
            } else {
                // Highs & Air: percussion flutter and delicate shimmer
                double h1 = std::sin(anim_phase_ * 1.6 + col * 0.55);
                double h2 = std::cos(anim_phase_ * 1.2 - col * 0.35 + 0.8);
                band_energy = 0.38 + 0.30 * h1 + 0.14 * h2;
            }

            target = envelope * band_energy * vol_scale * pan_mod;
            target = std::clamp(target, 0.05, 0.96);
        }

        // Ballistics: fast attack, smooth exponential analog decay
        if (target > bar_levels_[col]) {
            bar_levels_[col] += (target - bar_levels_[col]) * 0.50;
        } else {
            bar_levels_[col] += (target - bar_levels_[col]) * 0.16;
        }

        // Peak Hold
        if (bar_levels_[col] >= peak_levels_[col]) {
            peak_levels_[col] = bar_levels_[col];
        } else {
            peak_levels_[col] = std::max(bar_levels_[col], peak_levels_[col] - 0.018);
        }

        // Render column with floating peak cap
        double total_level = bar_levels_[col] * max_height;
        int peak_row = static_cast<int>(peak_levels_[col] * max_height);
        peak_row = std::clamp(peak_row, 0, max_height - 1);

        ftxui::Elements col_cells;
        for (int r = max_height - 1; r >= 0; --r) {
            double cell_fill = total_level - r;
            std::string glyph = " ";
            ftxui::Color cell_color = Theme::SecondaryBg;

            if (r == peak_row && r > 1 && cell_fill < 0.2) {
                glyph = "▔"; // floating peak cap
                cell_color = Theme::Accent;
            } else if (cell_fill >= 1.0) {
                glyph = "█";
            } else if (cell_fill > 0.0) {
                int block_idx = static_cast<int>(cell_fill * 8.0);
                block_idx = std::clamp(block_idx, 1, 8);
                glyph = blocks[block_idx];
            }

            // Warm atmospheric gradient conforming to design system
            if (glyph != " ") {
                if (cell_color != Theme::Accent) {
                    if (r >= 8) {
                        cell_color = Theme::Accent; // Peak terracotta: #d99178
                    } else if (r >= 5) {
                        cell_color = Theme::DimAccent; // Mid terracotta: #b56850
                    } else if (r >= 2) {
                        cell_color = ftxui::Color::RGB(95, 68, 60); // Warm umber
                    } else {
                        cell_color = ftxui::Color::RGB(55, 45, 42); // Warm charcoal base
                    }
                }
            }

            col_cells.push_back(ftxui::text(glyph) | ftxui::color(cell_color));
        }

        columns.push_back(ftxui::vbox(std::move(col_cells)));
    }

    return ftxui::hbox(std::move(columns)) | ftxui::center;
}

ftxui::Component FullscreenPlayer::GetComponent() {
    return ftxui::Renderer([this]() -> ftxui::Element {
        // Panning indicator text for header
        double pan_bias = std::sin(pan_phase_) * 0.35;
        std::string pan_str = "[pan: center]";
        if (pan_bias < -0.15) pan_str = "[pan: ◀ left]";
        else if (pan_bias > 0.15) pan_str = "[pan: right ▶]";

        // 1. Top Header Bar with macOS Window Dots
        auto top_bar = ftxui::hbox({
            Theme::window_dots(),
            ftxui::text("  ~/ymcli/visualizer") | ftxui::bold | ftxui::color(Theme::Accent),
            ftxui::text(" ▊") | ftxui::color(Theme::PrimaryDark),
            ftxui::text("  ") | ftxui::color(Theme::BorderLight),
            ftxui::text(state_.has_track && !state_.is_paused ? "● streaming" : "● paused") | ftxui::color(state_.has_track && !state_.is_paused ? Theme::Success : Theme::Accent),
            ftxui::text("  " + pan_str) | ftxui::color(Theme::KeywordBlue),
            ftxui::filler(),
            ftxui::text("[Shift+F / Esc] Exit  ") | ftxui::color(Theme::TextTertiary)
        }) | ftxui::bgcolor(Theme::SecondaryBg);

        // 2. Center Visualizer Area
        int num_bars = 52;
        int vis_height = 12;
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
