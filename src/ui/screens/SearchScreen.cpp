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
                ftxui::separator() | ftxui::color(Theme::Border),
                ftxui::text(" Searching YouTube Music...") | ftxui::center | ftxui::color(Theme::DimAccent) | ftxui::flex
            }) | ftxui::bgcolor(Theme::Background);
        }

        ftxui::Elements list_elements;
        size_t count = 0;

        if (tab_index_ == 0) {
            count = songs_.size();
            if (count == 0) {
                list_elements.push_back(
                    ftxui::text("No song results. Type a query in the search bar above (press /).")
                    | ftxui::color(Theme::TextTertiary) | ftxui::center
                );
            }
            for (size_t i = 0; i < count; ++i) {
                auto& s = songs_[i];
                bool is_sel = (static_cast<int>(i) == selected_item_);

                auto cursor = ftxui::text(is_sel ? "▸ " : "  ") | ftxui::color(is_sel ? Theme::Accent : Theme::TextTertiary);
                auto title_el = ftxui::text(s.title) | ftxui::bold | ftxui::color(is_sel ? Theme::TextPrimary : Theme::TextSecondary) | ftxui::flex;
                auto artist_el = ftxui::text(s.artist) | ftxui::color(Theme::TextSecondary) | ftxui::flex;
                auto dur_el = ftxui::text(s.duration.empty() ? "--:--" : s.duration) | ftxui::color(Theme::TextTertiary);

                auto row = ftxui::hbox({
                    cursor,
                    title_el,
                    artist_el,
                    dur_el
                });

                if (is_sel) {
                    row = row | ftxui::bgcolor(Theme::Elevated);
                }

                list_elements.push_back(row);
            }
        } else if (tab_index_ == 1) {
            count = albums_.size();
            for (size_t i = 0; i < count; ++i) {
                auto& a = albums_[i];
                bool is_sel = (static_cast<int>(i) == selected_item_);
                auto row = ftxui::hbox({
                    ftxui::text(is_sel ? "▸ " : "  ") | ftxui::color(Theme::Accent),
                    ftxui::text(a.title) | ftxui::bold | ftxui::flex,
                    ftxui::text(a.artist) | ftxui::color(Theme::TextSecondary) | ftxui::flex,
                    ftxui::text(a.year) | ftxui::color(Theme::TextTertiary)
                });
                if (is_sel) row = row | ftxui::bgcolor(Theme::Elevated);
                list_elements.push_back(row);
            }
        }

        return ftxui::vbox({
            tab_toggle_->Render(),
            ftxui::separator() | ftxui::color(Theme::Border),
            ftxui::vbox(list_elements) | ftxui::yframe | ftxui::flex
        }) | ftxui::bgcolor(Theme::Background);
    });

    component_ |= ftxui::CatchEvent([this](ftxui::Event event) {
        size_t max_count = songs_.size();
        if (tab_index_ == 1) max_count = albums_.size();

        if (event == ftxui::Event::Character('j') || event == ftxui::Event::ArrowDown) {
            if (max_count > 0) {
                selected_item_ = std::min(static_cast<int>(max_count - 1), selected_item_ + 1);
            }
            return true;
        }
        if (event == ftxui::Event::Character('k') || event == ftxui::Event::ArrowUp) {
            selected_item_ = std::max(0, selected_item_ - 1);
            return true;
        }
        return false;
    });
}

ftxui::Component SearchScreen::GetComponent() { return component_; }
void SearchScreen::SetLoading(bool loading) { is_loading_ = loading; }
void SearchScreen::SetSongs(const std::vector<SearchResultSong>& songs) { songs_ = songs; selected_item_ = 0; }
void SearchScreen::SetAlbums(const std::vector<SearchResultAlbum>& albums) { albums_ = albums; selected_item_ = 0; }
void SearchScreen::SetArtists(const std::vector<SearchResultArtist>& artists) { artists_ = artists; selected_item_ = 0; }
void SearchScreen::SetPlaylists(const std::vector<SearchResultPlaylist>& playlists) { playlists_ = playlists; selected_item_ = 0; }

} // namespace ymcli::ui::screens
