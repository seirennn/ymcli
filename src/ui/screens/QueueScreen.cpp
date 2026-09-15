#include "QueueScreen.hpp"
#include "../Theme.hpp"
#include "../../util/Format.hpp"
#include <ftxui/dom/elements.hpp>

namespace ymcli::ui::screens {

QueueScreen::QueueScreen(JumpTrackCallback jump_cb, RemoveTrackCallback remove_cb)
    : jump_cb_(std::move(jump_cb)), remove_cb_(std::move(remove_cb))
{
    auto dummy = ftxui::Container::Vertical({});

    component_ = ftxui::Renderer(dummy, [this] {
        ftxui::Elements rows;

        auto header_row = ftxui::hbox({
            ftxui::text("  ") | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 3),
            ftxui::text("TITLE") | ftxui::bold | ftxui::color(Theme::TextSecondary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 36),
            ftxui::text("ARTIST") | ftxui::bold | ftxui::color(Theme::TextSecondary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 24),
            ftxui::text("ALBUM") | ftxui::bold | ftxui::color(Theme::TextSecondary) | ftxui::flex,
            ftxui::text("TIME") | ftxui::bold | ftxui::color(Theme::TextSecondary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 8)
        }) | ftxui::bgcolor(Theme::SecondaryBg);

        rows.push_back(header_row);
        rows.push_back(ftxui::separator() | ftxui::color(Theme::Border));

        if (tracks_.empty()) {
            rows.push_back(
                ftxui::text("Play queue is empty. Search for songs and press Enter or 'a' to queue.")
                | ftxui::color(Theme::TextTertiary) | ftxui::center
            );
        } else {
            for (size_t i = 0; i < tracks_.size(); ++i) {
                const auto& song = tracks_[i];
                bool is_playing = (i == current_index_);
                bool is_sel = (static_cast<int>(i) == selected_);

                std::string cursor = is_playing ? "► " : (is_sel ? "› " : "  ");
                auto cursor_color = is_playing ? ftxui::color(Theme::PlayingIndicator) : (is_sel ? ftxui::color(Theme::Accent) : ftxui::color(Theme::TextTertiary));

                auto row = ftxui::hbox({
                    ftxui::text(cursor) | cursor_color | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 3),
                    ftxui::text(ymcli::truncate(song.title, 34)) | ftxui::bold | ftxui::color(is_playing ? Theme::PlayingIndicator : (is_sel ? Theme::TextPrimary : Theme::TextSecondary)) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 36),
                    ftxui::text(ymcli::truncate(song.artist, 22)) | ftxui::color(Theme::TextSecondary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 24),
                    ftxui::text(ymcli::truncate(song.album, 28)) | ftxui::color(Theme::TextTertiary) | ftxui::flex,
                    ftxui::text(song.duration_text.empty() ? "--:--" : song.duration_text) | ftxui::color(Theme::TextTertiary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 8)
                });

                if (is_sel) {
                    row = row | ftxui::bgcolor(Theme::Elevated);
                }

                rows.push_back(row);
            }
        }

        return ftxui::vbox({
            ftxui::hbox({
                ftxui::text("PLAY QUEUE") | ftxui::bold | ftxui::color(Theme::Accent),
                ftxui::text(" (" + std::to_string(tracks_.size()) + " tracks)") | ftxui::color(Theme::TextTertiary),
                ftxui::filler(),
                ftxui::text("[Enter] jump  [d] remove") | ftxui::color(Theme::TextTertiary)
            }),
            ftxui::separator() | ftxui::color(Theme::Border),
            ftxui::vbox(std::move(rows)) | ftxui::yframe | ftxui::flex
        }) | ftxui::bgcolor(Theme::Background);
    });

    component_ |= ftxui::CatchEvent([this](ftxui::Event event) {
        if (tracks_.empty()) return false;

        if (event == ftxui::Event::Character('j') || event == ftxui::Event::ArrowDown) {
            selected_ = std::min(static_cast<int>(tracks_.size() - 1), selected_ + 1);
            return true;
        }
        if (event == ftxui::Event::Character('k') || event == ftxui::Event::ArrowUp) {
            selected_ = std::max(0, selected_ - 1);
            return true;
        }
        if (event == ftxui::Event::Return) {
            if (jump_cb_ && selected_ >= 0 && selected_ < static_cast<int>(tracks_.size())) {
                jump_cb_(selected_);
            }
            return true;
        }
        if (event == ftxui::Event::Character('d')) {
            if (remove_cb_ && selected_ >= 0 && selected_ < static_cast<int>(tracks_.size())) {
                remove_cb_(selected_);
            }
            return true;
        }
        return false;
    });
}

ftxui::Component QueueScreen::GetComponent() { return component_; }

void QueueScreen::UpdateQueue(const std::vector<Track>& tracks, size_t current_index) {
    tracks_ = tracks;
    current_index_ = current_index;
    if (selected_ >= static_cast<int>(tracks_.size())) {
        selected_ = std::max(0, static_cast<int>(tracks_.size()) - 1);
    }
}

} // namespace ymcli::ui::screens
