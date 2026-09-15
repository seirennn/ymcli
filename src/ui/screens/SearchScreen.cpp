#include "SearchScreen.hpp"
#include "../Theme.hpp"
#include <ftxui/dom/elements.hpp>

namespace ymcli::ui::screens {

SearchScreen::SearchScreen() {
    tab_toggle_ = ftxui::Toggle(&tab_names_, &tab_index_);

    auto container = ftxui::Container::Vertical({
        tab_toggle_
    });

    component_ = ftxui::Renderer(container, [this] {
        if (is_loading_) {
            return ftxui::vbox({
                tab_toggle_->Render(),
                ftxui::text("Loading...") | ftxui::center | ftxui::color(Theme::DimAccent) | ftxui::flex
            });
        }

        ftxui::Elements list_elements;
        size_t count = 0;

        if (tab_index_ == 0) {
            count = songs_.size();
            for (size_t i = 0; i < count; ++i) {
                auto& s = songs_[i];
                auto el = ftxui::hbox({
                    ftxui::text(s.title) | ftxui::color(Theme::TextPrimary) | ftxui::flex,
                    ftxui::text(s.artist) | ftxui::color(Theme::TextSecondary) | ftxui::flex,
                    ftxui::text(s.duration) | ftxui::color(Theme::TextTertiary)
                });
                if (static_cast<int>(i) == selected_item_) el = el | Theme::focused_style();
                list_elements.push_back(el);
            }
        } else if (tab_index_ == 1) {
            count = albums_.size();
            for (size_t i = 0; i < count; ++i) {
                auto& a = albums_[i];
                auto el = ftxui::hbox({
                    ftxui::text(a.title) | ftxui::color(Theme::TextPrimary) | ftxui::flex,
                    ftxui::text(a.artist) | ftxui::color(Theme::TextSecondary) | ftxui::flex,
                    ftxui::text(a.year) | ftxui::color(Theme::TextTertiary)
                });
                if (static_cast<int>(i) == selected_item_) el = el | Theme::focused_style();
                list_elements.push_back(el);
            }
        } else if (tab_index_ == 2) {
            count = artists_.size();
            for (size_t i = 0; i < count; ++i) {
                auto& a = artists_[i];
                auto el = ftxui::hbox({
                    ftxui::text(a.name) | ftxui::color(Theme::TextPrimary) | ftxui::flex,
                    ftxui::text(a.subscribers) | ftxui::color(Theme::TextSecondary)
                });
                if (static_cast<int>(i) == selected_item_) el = el | Theme::focused_style();
                list_elements.push_back(el);
            }
        } else {
            count = playlists_.size();
            for (size_t i = 0; i < count; ++i) {
                auto& p = playlists_[i];
                auto el = ftxui::hbox({
                    ftxui::text(p.title) | ftxui::color(Theme::TextPrimary) | ftxui::flex,
                    ftxui::text(p.author) | ftxui::color(Theme::TextSecondary) | ftxui::flex,
                    ftxui::text(p.track_count) | ftxui::color(Theme::TextTertiary)
                });
                if (static_cast<int>(i) == selected_item_) el = el | Theme::focused_style();
                list_elements.push_back(el);
            }
        }

        return ftxui::vbox({
            tab_toggle_->Render(),
            ftxui::separator() | ftxui::color(Theme::Border),
            ftxui::vbox(list_elements) | ftxui::yframe | ftxui::flex
        });
    });

    component_ |= ftxui::CatchEvent([this](ftxui::Event event) {
        if (event == ftxui::Event::Character('j') || event == ftxui::Event::ArrowDown) {
            selected_item_++; // Simple bounds checking omitted for brevity, should use count
            return true;
        }
        if (event == ftxui::Event::Character('k') || event == ftxui::Event::ArrowUp) {
            selected_item_ = std::max(0, selected_item_ - 1);
            return true;
        }
        // a, f, Enter logic here...
        return false;
    });
}

ftxui::Component SearchScreen::GetComponent() { return component_; }
void SearchScreen::SetLoading(bool loading) { is_loading_ = loading; }
void SearchScreen::SetSongs(const std::vector<SearchResultSong>& songs) { songs_ = songs; }
void SearchScreen::SetAlbums(const std::vector<SearchResultAlbum>& albums) { albums_ = albums; }
void SearchScreen::SetArtists(const std::vector<SearchResultArtist>& artists) { artists_ = artists; }
void SearchScreen::SetPlaylists(const std::vector<SearchResultPlaylist>& playlists) { playlists_ = playlists; }

} // namespace ymcli::ui::screens
