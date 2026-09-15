#include "ArtistScreen.hpp"
#include "../Theme.hpp"
#include "../../util/Format.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>

namespace ymcli::ui::screens {

ArtistScreen::ArtistScreen(PlayTracksCallback play_cb,
                           OpenAlbumCallback album_cb,
                           EnqueueCallback enqueue_cb,
                           AddToPlaylistCallback add_to_pl_cb,
                           FavoriteCallback fav_cb,
                           BackCallback back_cb)
    : play_cb_(std::move(play_cb)),
      album_cb_(std::move(album_cb)),
      enqueue_cb_(std::move(enqueue_cb)),
      add_to_pl_cb_(std::move(add_to_pl_cb)),
      fav_cb_(std::move(fav_cb)),
      back_cb_(std::move(back_cb))
{
    auto dummy = ftxui::Container::Vertical({});

    component_ = ftxui::Renderer(dummy, [this] {
        ftxui::Elements song_elements;
        for (size_t i = 0; i < songs_.size(); ++i) {
            const auto& s = songs_[i];
            bool is_sel = (in_songs_ && static_cast<int>(i) == selected_song_);

            auto row = ftxui::hbox({
                ftxui::text(is_sel ? "› " : "  ") | ftxui::color(is_sel ? Theme::Accent : Theme::TextTertiary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 3),
                ftxui::text(std::to_string(i + 1) + ". ") | ftxui::color(Theme::TextTertiary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 4),
                ftxui::text(ymcli::truncate(s.title, 36)) | ftxui::bold | ftxui::color(is_sel ? Theme::TextPrimary : Theme::TextSecondary) | ftxui::flex,
                ftxui::text(s.duration_text.empty() ? "--:--" : s.duration_text) | ftxui::color(Theme::TextTertiary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 8)
            });

            if (is_sel) {
                row = row | ftxui::bgcolor(Theme::Elevated);
            }
            song_elements.push_back(row);
        }

        ftxui::Elements album_elements;
        for (size_t i = 0; i < albums_.size(); ++i) {
            const auto& a = albums_[i];
            bool is_sel = (!in_songs_ && static_cast<int>(i) == selected_album_);

            auto row = ftxui::hbox({
                ftxui::text(is_sel ? "› " : "  ") | ftxui::color(is_sel ? Theme::Accent : Theme::TextTertiary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 3),
                ftxui::text(ymcli::truncate(a.title, 32)) | ftxui::bold | ftxui::color(is_sel ? Theme::TextPrimary : Theme::TextSecondary) | ftxui::flex,
                ftxui::text(a.year) | ftxui::color(Theme::TextTertiary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 8)
            });

            if (is_sel) {
                row = row | ftxui::bgcolor(Theme::Elevated);
            }
            album_elements.push_back(row);
        }

        std::string meta = subs_.empty() ? "Artist" : subs_;

        return ftxui::vbox({
            ftxui::vbox({
                Theme::heading(name_.empty() ? "Artist" : name_),
                ftxui::hbox({
                    Theme::subtext(meta),
                    ftxui::filler(),
                    ftxui::text("[Tab] Toggle Songs/Albums  [Enter] Select  [Esc] Back") | ftxui::color(Theme::TextTertiary)
                })
            }),
            ftxui::separator() | ftxui::color(Theme::Border),
            ftxui::hbox({
                ftxui::vbox({
                    ftxui::text("TOP TRACKS") | ftxui::bold | ftxui::color(in_songs_ ? Theme::Accent : Theme::TextSecondary),
                    ftxui::separator() | ftxui::color(Theme::Border),
                    ftxui::vbox(std::move(song_elements)) | ftxui::yframe | ftxui::flex
                }) | ftxui::flex,
                ftxui::separator() | ftxui::color(Theme::Border),
                ftxui::vbox({
                    ftxui::text("DISCOGRAPHY / ALBUMS") | ftxui::bold | ftxui::color(!in_songs_ ? Theme::Accent : Theme::TextSecondary),
                    ftxui::separator() | ftxui::color(Theme::Border),
                    ftxui::vbox(std::move(album_elements)) | ftxui::yframe | ftxui::flex
                }) | ftxui::flex
            }) | ftxui::flex
        }) | ftxui::bgcolor(Theme::Background);
    });

    component_ |= ftxui::CatchEvent([this](ftxui::Event event) {
        if (event == ftxui::Event::Escape) {
            if (back_cb_) {
                back_cb_();
                return true;
            }
        }
        if (event == ftxui::Event::Tab) {
            in_songs_ = !in_songs_;
            return true;
        }

        if (in_songs_) {
            if (event == ftxui::Event::Character('j') || event == ftxui::Event::ArrowDown) {
                if (!songs_.empty()) selected_song_ = std::min(static_cast<int>(songs_.size() - 1), selected_song_ + 1);
                return true;
            }
            if (event == ftxui::Event::Character('k') || event == ftxui::Event::ArrowUp) {
                selected_song_ = std::max(0, selected_song_ - 1);
                return true;
            }
            if (event == ftxui::Event::Return) {
                if (play_cb_ && selected_song_ >= 0 && selected_song_ < static_cast<int>(songs_.size())) {
                    play_cb_(songs_, selected_song_);
                }
                return true;
            }
            if (event == ftxui::Event::Character('a')) {
                if (enqueue_cb_ && selected_song_ >= 0 && selected_song_ < static_cast<int>(songs_.size())) {
                    enqueue_cb_(songs_[selected_song_]);
                }
                return true;
            }
            if (event == ftxui::Event::Character('l') || event == ftxui::Event::Character('+')) {
                if (add_to_pl_cb_ && selected_song_ >= 0 && selected_song_ < static_cast<int>(songs_.size())) {
                    add_to_pl_cb_(songs_[selected_song_]);
                }
                return true;
            }
            if (event == ftxui::Event::Character('f')) {
                if (fav_cb_ && selected_song_ >= 0 && selected_song_ < static_cast<int>(songs_.size())) {
                    fav_cb_(songs_[selected_song_]);
                }
                return true;
            }
        } else {
            if (event == ftxui::Event::Character('j') || event == ftxui::Event::ArrowDown) {
                if (!albums_.empty()) selected_album_ = std::min(static_cast<int>(albums_.size() - 1), selected_album_ + 1);
                return true;
            }
            if (event == ftxui::Event::Character('k') || event == ftxui::Event::ArrowUp) {
                selected_album_ = std::max(0, selected_album_ - 1);
                return true;
            }
            if (event == ftxui::Event::Return) {
                if (album_cb_ && selected_album_ >= 0 && selected_album_ < static_cast<int>(albums_.size())) {
                    album_cb_(albums_[selected_album_].browse_id);
                }
                return true;
            }
        }
        return false;
    });
}

ftxui::Component ArtistScreen::GetComponent() { return component_; }

void ArtistScreen::SetArtist(const Artist& artist) {
    name_ = artist.name;
    subs_ = artist.subscriber_count;
    songs_ = artist.top_songs;
    for (auto& s : songs_) {
        if (s.artist.empty()) s.artist = artist.name;
    }
    albums_ = artist.albums;
    selected_song_ = 0;
    selected_album_ = 0;
}

} // namespace ymcli::ui::screens
