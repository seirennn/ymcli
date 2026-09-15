#include "AlbumScreen.hpp"
#include "../Theme.hpp"
#include "../../util/Format.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>

namespace ymcli::ui::screens {

AlbumScreen::AlbumScreen(PlayTracksCallback play_cb,
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
    auto dummy = ftxui::Container::Vertical({});

    component_ = ftxui::Renderer(dummy, [this] {
        ftxui::Elements track_elements;

        for (size_t i = 0; i < tracks_.size(); ++i) {
            const auto& t = tracks_[i];
            bool is_sel = (static_cast<int>(i) == selected_);

            auto row = ftxui::hbox({
                ftxui::text(is_sel ? "› " : "  ") | ftxui::color(is_sel ? Theme::Accent : Theme::TextTertiary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 3),
                ftxui::text(std::to_string(i + 1) + ". ") | ftxui::color(Theme::TextTertiary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 4),
                ftxui::text(ymcli::truncate(t.title, 40)) | ftxui::bold | ftxui::color(is_sel ? Theme::TextPrimary : Theme::TextSecondary) | ftxui::flex,
                ftxui::text(t.duration_text.empty() ? "--:--" : t.duration_text) | ftxui::color(Theme::TextTertiary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 8)
            });

            if (is_sel) {
                row = row | ftxui::bgcolor(Theme::Elevated);
            }

            track_elements.push_back(row);
        }

        std::string meta = artist_;
        if (!year_.empty()) meta += " • " + year_;
        meta += " • " + std::to_string(tracks_.size()) + " tracks";

        return ftxui::vbox({
            ftxui::vbox({
                Theme::heading(title_.empty() ? "Album" : title_),
                ftxui::hbox({
                    Theme::subtext(meta),
                    ftxui::filler(),
                    ftxui::text("[Enter] Play  [P] Play All  [a] Add  [l] Save to List  [Esc] Back") | ftxui::color(Theme::TextTertiary)
                })
            }),
            ftxui::separator() | ftxui::color(Theme::Border),
            ftxui::vbox(std::move(track_elements)) | ftxui::yframe | ftxui::flex
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

ftxui::Component AlbumScreen::GetComponent() { return component_; }

void AlbumScreen::SetAlbum(const Album& album) {
    title_ = album.title;
    artist_ = album.artist;
    year_ = album.year;
    tracks_ = album.tracks;
    for (auto& t : tracks_) {
        if (t.album.empty()) t.album = album.title;
        if (t.artist.empty()) t.artist = album.artist;
    }
    selected_ = 0;
}

} // namespace ymcli::ui::screens
