#include "FavoritesScreen.hpp"
#include "../Theme.hpp"
#include "../../util/Format.hpp"
#include <ftxui/dom/elements.hpp>

namespace ymcli::ui::screens {

FavoritesScreen::FavoritesScreen(PlayTrackCallback play_cb, UnfavoriteCallback unfav_cb)
    : play_cb_(std::move(play_cb)), unfav_cb_(std::move(unfav_cb))
{
    auto dummy = ftxui::Container::Vertical({});

    component_ = ftxui::Renderer(dummy, [this] {
        ftxui::Elements rows;

        auto header_row = ftxui::hbox({
            ftxui::text("  ") | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 3),
            ftxui::text("TITLE") | ftxui::bold | ftxui::color(Theme::TextSecondary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 36),
            ftxui::text("ARTIST") | ftxui::bold | ftxui::color(Theme::TextSecondary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 24),
            ftxui::text("ALBUM") | ftxui::bold | ftxui::color(Theme::TextSecondary) | ftxui::flex
        }) | ftxui::bgcolor(Theme::SecondaryBg);

        rows.push_back(header_row);
        rows.push_back(ftxui::separator() | ftxui::color(Theme::Border));

        if (favorites_.empty()) {
            rows.push_back(
                ftxui::text("No favorite songs yet. Press 'f' on any song to add it to your favorites.")
                | ftxui::color(Theme::TextTertiary) | ftxui::center
            );
        } else {
            for (size_t i = 0; i < favorites_.size(); ++i) {
                const auto& song = favorites_[i];
                bool is_sel = (static_cast<int>(i) == selected_);

                auto row = ftxui::hbox({
                    ftxui::text(is_sel ? "▸ " : "  ") | ftxui::color(is_sel ? Theme::Accent : Theme::TextTertiary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 3),
                    ftxui::text(ymcli::truncate(song.title, 34)) | ftxui::bold | ftxui::color(is_sel ? Theme::TextPrimary : Theme::TextSecondary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 36),
                    ftxui::text(ymcli::truncate(song.artist, 22)) | ftxui::color(Theme::TextSecondary) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 24),
                    ftxui::text(ymcli::truncate(song.album, 28)) | ftxui::color(Theme::TextTertiary) | ftxui::flex
                });

                if (is_sel) {
                    row = row | ftxui::bgcolor(Theme::Elevated);
                }

                rows.push_back(row);
            }
        }

        return ftxui::vbox({
            ftxui::hbox({
                ftxui::text("FAVORITE TRACKS") | ftxui::bold | ftxui::color(Theme::Accent),
                ftxui::text(" (" + std::to_string(favorites_.size()) + " saved)") | ftxui::color(Theme::TextTertiary)
            }),
            ftxui::separator() | ftxui::color(Theme::Border),
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
                play_cb_(favorites_[selected_]);
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
