#include "PlaylistScreen.hpp"
#include "../Theme.hpp"
#include "../../util/Format.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>

namespace ymcli::ui::screens {

PlaylistScreen::PlaylistScreen(PlayTracksCallback play_cb,
                               EnqueueCallback enqueue_cb,
                               AddToPlaylistCallback add_to_pl_cb,
                               FavoriteCallback fav_cb,
                               BackCallback back_cb)
    : play_cb_(std::move(play_cb)),
      enqueue_cb_(std::move(enqueue_cb)),
      add_to_pl_cb_(std::move(add_to_pl_cb)),
      fav_cb_(std::move(fav_cb)),
      back_cb_(std::move(back_cb))
{
    component_ = ftxui::Renderer([this](bool focused) {
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

        if (is_loading_) {
            rows.push_back(
                ftxui::vbox({
                    ftxui::text(""),
                    ftxui::text("Loading playlist tracks from YouTube Music...") | ftxui::color(Theme::Accent) | ftxui::center,
                    ftxui::text("")
                })
            );
        } else if (tracks_.empty()) {
            rows.push_back(
                ftxui::text("No tracks found in this playlist.")
                | ftxui::color(Theme::TextTertiary) | ftxui::center
            );
        } else {
            for (size_t i = 0; i < tracks_.size(); ++i) {
                const auto& track = tracks_[i];
                bool is_sel = (static_cast<int>(i) == selected_);

                auto cursor_text = is_sel ? "> " : "  ";
                auto cursor_color = is_sel ? ftxui::color(Theme::Accent) : ftxui::color(Theme::TextTertiary);

                std::string title_str = ymcli::truncate(track.title, 34);
                std::string artist_str = ymcli::truncate(track.artist, 22);
                std::string album_str = ymcli::truncate(track.album.empty() ? title_ : track.album, 28);
                std::string time_str = track.duration_text.empty() ? "--:--" : track.duration_text;

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

        std::string author_str = author_.empty() ? "playlist" : author_;
        std::string count_str = std::to_string(tracks_.size()) + " tracks";

        auto top_header = ftxui::vbox({
            ftxui::hbox({
                Theme::cmd_header("playlist", "--tracks", title_.empty() ? "untitled" : title_),
                ftxui::filler(),
                ftxui::text("● " + author_str) | ftxui::color(Theme::KeywordBlue),
                ftxui::text("  " + count_str) | ftxui::color(Theme::TextTertiary)
            }),
            ftxui::hbox({
                ftxui::text("[Enter] Play  [P] Play All  [A] Add All  [a] Add  [l] Save to List  [Esc] Back") | ftxui::color(Theme::TextTertiary)
            })
        });

        return ftxui::vbox({
            top_header,
            ftxui::separator() | ftxui::color(Theme::BorderLight),
            ftxui::vbox(std::move(rows)) | ftxui::yframe | ftxui::flex
        }) | ftxui::bgcolor(Theme::Background);
    });

    component_ |= ftxui::CatchEvent([this](ftxui::Event event) {
        if (event == ftxui::Event::Escape) {
            if (back_cb_) {
                back_cb_();
                return true;
            }
        }

        if (event == ftxui::Event::Character('j') || event == ftxui::Event::ArrowDown) {
            if (!tracks_.empty()) {
                selected_ = std::min(static_cast<int>(tracks_.size() - 1), selected_ + 1);
            }
            return true;
        }
        if (event == ftxui::Event::Character('k') || event == ftxui::Event::ArrowUp) {
            selected_ = std::max(0, selected_ - 1);
            return true;
        }
        if (event == ftxui::Event::Return) {
            if (play_cb_ && selected_ >= 0 && selected_ < static_cast<int>(tracks_.size())) {
                play_cb_(tracks_, selected_);
            }
            return true;
        }
        if (event == ftxui::Event::Character('P')) {
            if (play_cb_ && !tracks_.empty()) {
                play_cb_(tracks_, 0);
            }
            return true;
        }
        if (event == ftxui::Event::Character('A')) {
            if (enqueue_cb_ && !tracks_.empty()) {
                for (const auto& t : tracks_) {
                    enqueue_cb_(t);
                }
            }
            return true;
        }
        if (event == ftxui::Event::Character('a')) {
            if (enqueue_cb_ && selected_ >= 0 && selected_ < static_cast<int>(tracks_.size())) {
                enqueue_cb_(tracks_[selected_]);
            }
            return true;
        }
        if (event == ftxui::Event::Character('l') || event == ftxui::Event::Character('+')) {
            if (add_to_pl_cb_ && selected_ >= 0 && selected_ < static_cast<int>(tracks_.size())) {
                add_to_pl_cb_(tracks_[selected_]);
            }
            return true;
        }
        if (event == ftxui::Event::Character('f')) {
            if (fav_cb_ && selected_ >= 0 && selected_ < static_cast<int>(tracks_.size())) {
                fav_cb_(tracks_[selected_]);
            }
            return true;
        }
        return false;
    });
}

ftxui::Component PlaylistScreen::GetComponent() { return component_; }

void PlaylistScreen::SetPlaylist(const Playlist& playlist) {
    title_ = playlist.title.empty() ? title_ : playlist.title;
    author_ = playlist.author.empty() ? author_ : playlist.author;
    track_count_ = playlist.track_count > 0 ? playlist.track_count : static_cast<int>(playlist.tracks.size());
    tracks_ = playlist.tracks;
    is_loading_ = false;
    selected_ = 0;
}

void PlaylistScreen::SetLoading(bool is_loading) {
    is_loading_ = is_loading;
    if (is_loading) {
        tracks_.clear();
        selected_ = 0;
    }
}

void PlaylistScreen::SetHeader(const std::string& title, const std::string& author) {
    title_ = title;
    author_ = author;
}

} // namespace ymcli::ui::screens
