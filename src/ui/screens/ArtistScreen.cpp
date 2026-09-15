#include "ArtistScreen.hpp"
#include "../Theme.hpp"
#include <ftxui/dom/elements.hpp>

namespace ymcli::ui::screens {

ArtistScreen::ArtistScreen() {
    component_ = ftxui::Renderer([this] {
        auto header = ftxui::vbox({
            Theme::heading(name_),
            Theme::subtext(subs_)
        });

        ftxui::Elements s_elements, a_elements;
        for (size_t i = 0; i < songs_.size(); ++i) {
            auto el = ftxui::text(songs_[i].title) | ftxui::color(Theme::TextPrimary);
            if (in_songs_ && static_cast<int>(i) == selected_song_) el = el | Theme::focused_style();
            s_elements.push_back(el);
        }
        for (size_t i = 0; i < albums_.size(); ++i) {
            auto el = ftxui::hbox({
                ftxui::text(albums_[i].title) | ftxui::color(Theme::TextPrimary) | ftxui::flex,
                ftxui::text(albums_[i].year) | ftxui::color(Theme::TextTertiary)
            });
            if (!in_songs_ && static_cast<int>(i) == selected_album_) el = el | Theme::focused_style();
            a_elements.push_back(el);
        }

        return ftxui::vbox({
            header,
            ftxui::separator() | ftxui::color(Theme::Border),
            Theme::heading("Top Songs"),
            ftxui::vbox(s_elements) | ftxui::yframe,
            ftxui::separator() | ftxui::color(Theme::Border),
            Theme::heading("Albums"),
            ftxui::vbox(a_elements) | ftxui::yframe | ftxui::flex
        });
    });

    component_ |= ftxui::CatchEvent([this](ftxui::Event event) {
        if (event == ftxui::Event::Character('j')) {
            if (in_songs_) selected_song_++; else selected_album_++;
            return true;
        }
        if (event == ftxui::Event::Character('k')) {
            if (in_songs_) selected_song_--; else selected_album_--;
            return true;
        }
        if (event == ftxui::Event::Tab) { in_songs_ = !in_songs_; return true; }
        return false;
    });
}

ftxui::Component ArtistScreen::GetComponent() { return component_; }
void ArtistScreen::SetData(const std::string& name, const std::string& subs, const std::vector<ArtistSong>& songs, const std::vector<ArtistAlbum>& albums) {
    name_ = name; subs_ = subs; songs_ = songs; albums_ = albums; selected_song_ = 0; selected_album_ = 0;
}

} // namespace ymcli::ui::screens
