#include "LibraryScreen.hpp"
#include "../Theme.hpp"
#include "../../util/Format.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>

namespace ymcli::ui::screens {

LibraryScreen::LibraryScreen(OpenPlaylistCallback open_cb,
                             CreatePlaylistCallback create_cb,
                             DeletePlaylistCallback delete_cb)
    : open_cb_(std::move(open_cb)),
      create_cb_(std::move(create_cb)),
      delete_cb_(std::move(delete_cb))
{
    ftxui::InputOption input_opt;
    input_opt.multiline = false;
    input_comp_ = ftxui::Input(&new_playlist_name_, "Playlist name...", input_opt);

    auto container = ftxui::Container::Vertical({
        input_comp_
    });

    component_ = ftxui::Renderer(container, [this] {
        ftxui::Elements rows;

        auto header_row = ftxui::hbox({
            ftxui::text("  ") | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 3),
            ftxui::text("PLAYLIST NAME") | ftxui::bold | ftxui::color(Theme::TextSecondary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 40),
            ftxui::text("TYPE") | ftxui::bold | ftxui::color(Theme::TextSecondary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 24),
            ftxui::text("TRACKS") | ftxui::bold | ftxui::color(Theme::TextSecondary) | ftxui::flex
        }) | ftxui::bgcolor(Theme::SecondaryBg);

        rows.push_back(header_row);
        rows.push_back(ftxui::separator() | ftxui::color(Theme::Border));

        if (playlists_.empty()) {
            rows.push_back(
                ftxui::text("No playlists found. Press 'n' to create a new local playlist.")
                | ftxui::color(Theme::TextTertiary) | ftxui::center
            );
        } else {
            for (size_t i = 0; i < playlists_.size(); ++i) {
                const auto& pl = playlists_[i];
                bool is_sel = (static_cast<int>(i) == selected_);
                bool is_local = (pl.playlist_id.rfind("local:", 0) == 0);

                auto cursor_text = is_sel ? "› " : "  ";
                auto cursor_color = is_sel ? ftxui::color(Theme::Accent) : ftxui::color(Theme::TextTertiary);

                std::string title_str = ymcli::truncate(pl.title, 38);
                std::string type_str = is_local ? "Local Playlist" : (pl.author.empty() ? "YouTube Music" : ymcli::truncate(pl.author, 22));
                std::string count_str = std::to_string(pl.track_count) + " tracks";

                auto row = ftxui::hbox({
                    ftxui::text(cursor_text) | cursor_color | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 3),
                    ftxui::text(title_str) | ftxui::bold | ftxui::color(is_sel ? Theme::TextPrimary : Theme::TextSecondary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 40),
                    ftxui::text(type_str) | ftxui::color(is_local ? Theme::DimAccent : Theme::TextSecondary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 24),
                    ftxui::text(count_str) | ftxui::color(Theme::TextTertiary) | ftxui::flex
                });

                if (is_sel) {
                    row = row | ftxui::bgcolor(Theme::Elevated);
                }

                rows.push_back(row);
            }
        }

        ftxui::Elements top_elements;
        top_elements.push_back(
            ftxui::hbox({
                ftxui::text("PLAYLISTS & LIBRARY") | ftxui::bold | ftxui::color(Theme::Accent),
                ftxui::text(" (" + std::to_string(playlists_.size()) + " total)") | ftxui::color(Theme::TextTertiary),
                ftxui::filler(),
                ftxui::text("[Enter] Open  [n] New Playlist  [d] Delete") | ftxui::color(Theme::TextTertiary)
            })
        );

        if (is_creating_) {
            top_elements.push_back(ftxui::separator() | ftxui::color(Theme::Border));
            top_elements.push_back(
                ftxui::hbox({
                    ftxui::text("New Playlist: ") | ftxui::color(Theme::Accent) | ftxui::bold,
                    input_comp_->Render() | ftxui::flex,
                    ftxui::text(" [Enter: Confirm, Esc: Cancel]") | ftxui::color(Theme::TextTertiary)
                }) | ftxui::bgcolor(Theme::Elevated)
            );
        }

        top_elements.push_back(ftxui::separator() | ftxui::color(Theme::Border));
        top_elements.push_back(ftxui::vbox(std::move(rows)) | ftxui::yframe | ftxui::flex);

        return ftxui::vbox(std::move(top_elements)) | ftxui::bgcolor(Theme::Background);
    });

    component_ |= ftxui::CatchEvent([this](ftxui::Event event) {
        if (is_creating_) {
            if (event == ftxui::Event::Escape) {
                is_creating_ = false;
                new_playlist_name_.clear();
                return true;
            }
            if (event == ftxui::Event::Return) {
                if (!new_playlist_name_.empty() && create_cb_) {
                    create_cb_(new_playlist_name_);
                }
                is_creating_ = false;
                new_playlist_name_.clear();
                return true;
            }
            return input_comp_->OnEvent(event);
        }

        if (event == ftxui::Event::Character('j') || event == ftxui::Event::ArrowDown) {
            if (!playlists_.empty()) {
                selected_ = std::min(static_cast<int>(playlists_.size() - 1), selected_ + 1);
            }
            return true;
        }
        if (event == ftxui::Event::Character('k') || event == ftxui::Event::ArrowUp) {
            selected_ = std::max(0, selected_ - 1);
            return true;
        }
        if (event == ftxui::Event::Return) {
            if (open_cb_ && selected_ >= 0 && selected_ < static_cast<int>(playlists_.size())) {
                open_cb_(playlists_[selected_]);
            }
            return true;
        }
        if (event == ftxui::Event::Character('n')) {
            is_creating_ = true;
            new_playlist_name_.clear();
            input_comp_->TakeFocus();
            return true;
        }
        if (event == ftxui::Event::Character('d')) {
            if (delete_cb_ && selected_ >= 0 && selected_ < static_cast<int>(playlists_.size())) {
                const auto& pl = playlists_[selected_];
                if (pl.playlist_id.rfind("local:", 0) == 0) {
                    delete_cb_(pl.playlist_id);
                }
            }
            return true;
        }
        return false;
    });
}

ftxui::Component LibraryScreen::GetComponent() { return component_; }

void LibraryScreen::SetPlaylists(const std::vector<Playlist>& cloud_playlists,
                                 const std::vector<Playlist>& local_playlists)
{
    playlists_.clear();
    // Put cloud playlists first
    for (const auto& pl : cloud_playlists) {
        playlists_.push_back(pl);
    }
    // Then local playlists
    for (const auto& pl : local_playlists) {
        playlists_.push_back(pl);
    }

    if (selected_ >= static_cast<int>(playlists_.size())) {
        selected_ = std::max(0, static_cast<int>(playlists_.size()) - 1);
    }
}

} // namespace ymcli::ui::screens
