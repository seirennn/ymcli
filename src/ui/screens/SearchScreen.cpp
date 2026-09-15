#include "SearchScreen.hpp"
#include "../Theme.hpp"
#include "../../util/Format.hpp"
#include <ftxui/dom/elements.hpp>

namespace ymcli::ui::screens {

SearchScreen::SearchScreen(PlayTracksCallback play_cb, EnqueueCallback enqueue_cb, AddToPlaylistCallback add_to_pl_cb, FavoriteCallback favorite_cb)
    : play_cb_(std::move(play_cb)), enqueue_cb_(std::move(enqueue_cb)), add_to_pl_cb_(std::move(add_to_pl_cb)), favorite_cb_(std::move(favorite_cb))
{
    tab_toggle_ = ftxui::Toggle(&tab_names_, &tab_index_);

    auto container = ftxui::Container::Vertical({
        tab_toggle_
    });

    component_ = ftxui::Renderer(container, [this] {
        if (is_loading_) {
            return ftxui::vbox({
                tab_toggle_->Render(),
                ftxui::separator() | ftxui::color(Theme::Border),
                ftxui::text("Searching YouTube Music...") | ftxui::center | ftxui::color(Theme::Accent) | ftxui::flex
            }) | ftxui::bgcolor(Theme::Background);
        }

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

        if (tab_index_ == 0) {
            const auto& songs = results_.songs;
            if (songs.empty()) {
                rows.push_back(
                    ftxui::text("No song results. Type a query in the search bar above (press /).")
                    | ftxui::color(Theme::TextTertiary) | ftxui::center
                );
            } else {
                for (size_t i = 0; i < songs.size(); ++i) {
                    const auto& song = songs[i];
                    bool is_sel = (static_cast<int>(i) == selected_item_);

                    auto cursor_text = is_sel ? "› " : "  ";
                    auto cursor_color = is_sel ? ftxui::color(Theme::Accent) : ftxui::color(Theme::TextTertiary);

                    std::string title_str = ymcli::truncate(song.title, 34);
                    std::string artist_str = ymcli::truncate(song.artist, 22);
                    std::string album_str = ymcli::truncate(song.album, 28);
                    std::string time_str = song.duration_text.empty() ? "--:--" : song.duration_text;

                    auto row = ftxui::hbox({
                        ftxui::text(cursor_text) | cursor_color | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 3),
                        ftxui::text(title_str) | ftxui::bold | ftxui::color(is_sel ? Theme::TextPrimary : Theme::TextSecondary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 36),
                        ftxui::text(artist_str) | ftxui::color(Theme::TextSecondary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 24),
                        ftxui::text(album_str) | ftxui::color(Theme::TextTertiary) | ftxui::flex,
                        ftxui::text(time_str) | ftxui::color(Theme::TextTertiary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 8)
                    });

                    if (is_sel) {
                        row = row | ftxui::bgcolor(Theme::Elevated);
                    }

                    rows.push_back(row);
                }
            }
        }

        return ftxui::vbox({
            tab_toggle_->Render(),
            ftxui::separator() | ftxui::color(Theme::Border),
            ftxui::vbox(std::move(rows)) | ftxui::yframe | ftxui::flex
        }) | ftxui::bgcolor(Theme::Background);
    });

    component_ |= ftxui::CatchEvent([this](ftxui::Event event) {
        size_t count = results_.songs.size();

        if (event == ftxui::Event::Character('j') || event == ftxui::Event::ArrowDown) {
            if (count > 0) {
                selected_item_ = std::min(static_cast<int>(count - 1), selected_item_ + 1);
            }
            return true;
        }
        if (event == ftxui::Event::Character('k') || event == ftxui::Event::ArrowUp) {
            selected_item_ = std::max(0, selected_item_ - 1);
            return true;
        }
        if (event == ftxui::Event::Return) {
            if (tab_index_ == 0 && selected_item_ >= 0 && selected_item_ < static_cast<int>(results_.songs.size())) {
                if (play_cb_) {
                    play_cb_(results_.songs, selected_item_);
                }
                return true;
            }
        }
        if (event == ftxui::Event::Character('a')) {
            if (tab_index_ == 0 && selected_item_ >= 0 && selected_item_ < static_cast<int>(results_.songs.size())) {
                if (enqueue_cb_) {
                    enqueue_cb_(results_.songs[selected_item_]);
                }
                return true;
            }
        }
        if (event == ftxui::Event::Character('l') || event == ftxui::Event::Character('+')) {
            if (tab_index_ == 0 && selected_item_ >= 0 && selected_item_ < static_cast<int>(results_.songs.size())) {
                if (add_to_pl_cb_) {
                    add_to_pl_cb_(results_.songs[selected_item_]);
                }
                return true;
            }
        }
        if (event == ftxui::Event::Character('f')) {
            if (tab_index_ == 0 && selected_item_ >= 0 && selected_item_ < static_cast<int>(results_.songs.size())) {
                if (favorite_cb_) {
                    favorite_cb_(results_.songs[selected_item_]);
                }
                return true;
            }
        }
        return false;
    });
}

ftxui::Component SearchScreen::GetComponent() { return component_; }
void SearchScreen::SetLoading(bool loading) { is_loading_ = loading; }
void SearchScreen::SetResults(const SearchResults& results) { results_ = results; is_loading_ = false; selected_item_ = 0; }

} // namespace ymcli::ui::screens
