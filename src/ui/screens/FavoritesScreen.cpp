#include "FavoritesScreen.hpp"
#include "../Theme.hpp"
#include "../../util/Format.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>

namespace ymcli::ui::screens {

FavoritesScreen::FavoritesScreen(PlayTracksCallback play_cb,
                                 EnqueueCallback enqueue_cb,
                                 AddToPlaylistCallback add_to_pl_cb,
                                 UnfavoriteCallback unfav_cb)
    : play_cb_(std::move(play_cb)),
      enqueue_cb_(std::move(enqueue_cb)),
      add_to_pl_cb_(std::move(add_to_pl_cb)),
      unfav_cb_(std::move(unfav_cb))
{
    auto dummy = ftxui::Container::Vertical({});

    component_ = ftxui::Renderer(dummy, [this] {
        ftxui::Elements rows;

        auto header_row = ftxui::hbox({
            ftxui::text("  ") | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 3),
            ftxui::text("TITLE") | ftxui::bold | ftxui::color(Theme::TextTertiary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 36),
            ftxui::text("ARTIST") | ftxui::bold | ftxui::color(Theme::TextTertiary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 24),
            ftxui::text("ALBUM") | ftxui::bold | ftxui::color(Theme::TextTertiary) | ftxui::flex,
            ftxui::text("TIME") | ftxui::bold | ftxui::color(Theme::TextTertiary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 8)
        }) | ftxui::bgcolor(Theme::SecondaryBg);

        rows.push_back(header_row);
        rows.push_back(ftxui::separator() | ftxui::color(Theme::BorderLight));

        if (favorites_.empty()) {
            rows.push_back(
                ftxui::text("No saved songs found. Press 'f' on any song to save it here.")
                | ftxui::color(Theme::TextTertiary) | ftxui::center
            );
        } else {
            for (size_t i = 0; i < favorites_.size(); ++i) {
                const auto& song = favorites_[i];
                bool is_sel = (static_cast<int>(i) == selected_);

                auto cursor_text = is_sel ? "> " : "  ";
                auto cursor_color = is_sel ? ftxui::color(Theme::Accent) : ftxui::color(Theme::TextTertiary);

                std::string title_str = ymcli::truncate(song.title, 34);
                std::string artist_str = ymcli::truncate(song.artist, 22);
                std::string album_str = ymcli::truncate(song.album, 28);
                std::string time_str = song.duration_text.empty() ? "--:--" : song.duration_text;

                auto row = ftxui::hbox({
                    ftxui::text(cursor_text) | cursor_color | ftxui::bold | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 3),
                    ftxui::text(title_str) | ftxui::bold | ftxui::color(is_sel ? Theme::TextPrimary : Theme::TextSecondary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 36),
                    ftxui::text(artist_str) | ftxui::color(is_sel ? Theme::Accent : Theme::TextSecondary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 24),
                    ftxui::text(album_str) | ftxui::color(Theme::TextTertiary) | ftxui::flex,
                    ftxui::text(time_str) | ftxui::color(Theme::TextTertiary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 8)
                });

                if (is_sel) {
                    row = row | ftxui::bgcolor(Theme::Elevated);
                }

                rows.push_back(row);
            }
        }

        auto top_header = ftxui::hbox({
            Theme::cmd_header("favorites", "--saved", std::to_string(favorites_.size()) + " tracks"),
            ftxui::filler(),
            ftxui::text("[Enter] Play  [a] Queue  [l] Save to List  [f] Unfav") | ftxui::color(Theme::TextTertiary)
        });

        return ftxui::vbox({
            top_header,
            ftxui::separator() | ftxui::color(Theme::BorderLight),
            ftxui::vbox(std::move(rows)) | ftxui::yframe | ftxui::flex
        }) | ftxui::bgcolor(Theme::Background);
    });

    component_ |= ftxui::CatchEvent([this](ftxui::Event event) {
        if (favorites_.empty()) return false;

        if (event == ftxui::Event::Character('j') || event == ftxui::Event::ArrowDown) {
            selected_ = std::min(static_cast<int>(favorites_.size() - 1), selected_ + 1);
            return true;
        }
        if (event == ftxui::Event::Character('k') || event == ftxui::Event::ArrowUp) {
            selected_ = std::max(0, selected_ - 1);
            return true;
        }
        if (event == ftxui::Event::Return) {
            if (play_cb_ && selected_ >= 0 && selected_ < static_cast<int>(favorites_.size())) {
                play_cb_(favorites_, selected_);
            }
            return true;
        }
        if (event == ftxui::Event::Character('a')) {
            if (enqueue_cb_ && selected_ >= 0 && selected_ < static_cast<int>(favorites_.size())) {
                enqueue_cb_(favorites_[selected_]);
            }
            return true;
        }
        if (event == ftxui::Event::Character('l') || event == ftxui::Event::Character('+')) {
            if (add_to_pl_cb_ && selected_ >= 0 && selected_ < static_cast<int>(favorites_.size())) {
                add_to_pl_cb_(favorites_[selected_]);
            }
            return true;
        }
        if (event == ftxui::Event::Character('f')) {
            if (unfav_cb_ && selected_ >= 0 && selected_ < static_cast<int>(favorites_.size())) {
                unfav_cb_(favorites_[selected_]);
            }
            return true;
        }
        return false;
    });
}

ftxui::Component FavoritesScreen::GetComponent() { return component_; }

void FavoritesScreen::SetFavorites(const std::vector<Track>& favorites) {
    favorites_ = favorites;
    if (selected_ >= static_cast<int>(favorites_.size())) {
        selected_ = std::max(0, static_cast<int>(favorites_.size()) - 1);
    }
}

} // namespace ymcli::ui::screens
