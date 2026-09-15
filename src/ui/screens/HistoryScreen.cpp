#include "HistoryScreen.hpp"
#include "../Theme.hpp"
#include "../../util/Format.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>

namespace ymcli::ui::screens {

HistoryScreen::HistoryScreen(PlayTracksCallback play_cb,
                             EnqueueCallback enqueue_cb,
                             AddToPlaylistCallback add_to_pl_cb,
                             FavoriteCallback fav_cb)
    : play_cb_(std::move(play_cb)),
      enqueue_cb_(std::move(enqueue_cb)),
      add_to_pl_cb_(std::move(add_to_pl_cb)),
      fav_cb_(std::move(fav_cb))
{
    component_ = ftxui::Renderer([this](bool focused) {
        ftxui::Elements rows;

        auto header_row = ftxui::hbox({
            ftxui::text("  ") | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 3),
            ftxui::text("TITLE") | ftxui::bold | ftxui::color(Theme::TextTertiary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 36),
            ftxui::text("ARTIST") | ftxui::bold | ftxui::color(Theme::TextTertiary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 24),
            ftxui::text("ALBUM") | ftxui::bold | ftxui::color(Theme::TextTertiary) | ftxui::flex
        }) | ftxui::bgcolor(Theme::SecondaryBg);

        rows.push_back(header_row);
        rows.push_back(ftxui::separator() | ftxui::color(Theme::BorderLight));

        if (history_.empty()) {
            rows.push_back(
                ftxui::text("Play history is empty. Start playing songs to see them here.")
                | ftxui::color(Theme::TextTertiary) | ftxui::center
            );
        } else {
            for (size_t i = 0; i < history_.size(); ++i) {
                const auto& song = history_[i];
                bool is_sel = (static_cast<int>(i) == selected_);

                auto row = ftxui::hbox({
                    ftxui::text(is_sel ? "> " : "  ") | ftxui::bold | ftxui::color(is_sel ? (focused ? Theme::Accent : Theme::TextSecondary) : Theme::TextTertiary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 3),
                    ftxui::text(ymcli::truncate(song.title, 34)) | ftxui::bold | ftxui::color(is_sel ? Theme::TextPrimary : Theme::TextSecondary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 36),
                    ftxui::text(ymcli::truncate(song.artist, 22)) | ftxui::color(is_sel ? (focused ? Theme::Accent : Theme::TextSecondary) : Theme::TextSecondary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 24),
                    ftxui::text(ymcli::truncate(song.album, 28)) | ftxui::color(Theme::TextTertiary) | ftxui::flex
                });

                if (is_sel) {
                    row = row | ftxui::bgcolor(focused ? Theme::Elevated : Theme::Surface);
                }

                rows.push_back(row);
            }
        }

        auto top_header = ftxui::hbox({
            Theme::cmd_header("history", "--played", std::to_string(history_.size()) + " tracks"),
            ftxui::filler(),
            ftxui::text("[Enter] Play  [a] Queue  [l] Save to List  [f] Fav") | ftxui::color(Theme::TextTertiary)
        });

        return ftxui::vbox({
            top_header,
            ftxui::separator() | ftxui::color(Theme::BorderLight),
            ftxui::vbox(std::move(rows)) | ftxui::yframe | ftxui::flex
        }) | ftxui::bgcolor(Theme::Background);
    });

    component_ |= ftxui::CatchEvent([this](ftxui::Event event) {
        if (history_.empty()) return false;

        if (event == ftxui::Event::Character('j') || event == ftxui::Event::ArrowDown) {
            selected_ = std::min(static_cast<int>(history_.size() - 1), selected_ + 1);
            return true;
        }
        if (event == ftxui::Event::Character('k') || event == ftxui::Event::ArrowUp) {
            selected_ = std::max(0, selected_ - 1);
            return true;
        }
        if (event == ftxui::Event::Return) {
            if (play_cb_ && selected_ >= 0 && selected_ < static_cast<int>(history_.size())) {
                play_cb_(history_, selected_);
            }
            return true;
        }
        if (event == ftxui::Event::Character('a')) {
            if (enqueue_cb_ && selected_ >= 0 && selected_ < static_cast<int>(history_.size())) {
                enqueue_cb_(history_[selected_]);
            }
            return true;
        }
        if (event == ftxui::Event::Character('l') || event == ftxui::Event::Character('+')) {
            if (add_to_pl_cb_ && selected_ >= 0 && selected_ < static_cast<int>(history_.size())) {
                add_to_pl_cb_(history_[selected_]);
            }
            return true;
        }
        if (event == ftxui::Event::Character('f')) {
            if (fav_cb_ && selected_ >= 0 && selected_ < static_cast<int>(history_.size())) {
                fav_cb_(history_[selected_]);
            }
            return true;
        }
        return false;
    });
}

ftxui::Component HistoryScreen::GetComponent() { return component_; }

void HistoryScreen::SetHistory(const std::vector<Track>& history) {
    history_ = history;
    if (selected_ >= static_cast<int>(history_.size())) {
        selected_ = std::max(0, static_cast<int>(history_.size()) - 1);
    }
}

} // namespace ymcli::ui::screens
