#include "FavoritesScreen.hpp"
#include "../Theme.hpp"
#include <ftxui/dom/elements.hpp>

namespace ymcli::ui::screens {

FavoritesScreen::FavoritesScreen() {
    component_ = ftxui::Renderer([this] {
        ftxui::Elements elements;
        for (size_t i = 0; i < items_.size(); ++i) {
            auto& item = items_[i];
            auto el = ftxui::hbox({
                ftxui::text("♥ ") | ftxui::color(Theme::Accent),
                ftxui::text(item.title) | ftxui::color(Theme::TextPrimary) | ftxui::flex,
                ftxui::text(item.artist) | ftxui::color(Theme::TextSecondary)
            });
            if (static_cast<int>(i) == selected_) el = el | Theme::focused_style();
            elements.push_back(el);
        }

        return ftxui::vbox({
            Theme::heading("Favorites"),
            ftxui::text("[Enter] Play  [f] Unfavorite") | ftxui::color(Theme::DimAccent),
            ftxui::separator() | ftxui::color(Theme::Border),
            ftxui::vbox(elements) | ftxui::yframe | ftxui::flex
        });
    });

    component_ |= ftxui::CatchEvent([this](ftxui::Event event) {
        if (event == ftxui::Event::Character('j')) { selected_ = std::min((int)items_.size() - 1, selected_ + 1); return true; }
        if (event == ftxui::Event::Character('k')) { selected_ = std::max(0, selected_ - 1); return true; }
        return false;
    });
}

ftxui::Component FavoritesScreen::GetComponent() { return component_; }
void FavoritesScreen::SetFavorites(const std::vector<FavoriteItem>& items) { items_ = items; selected_ = 0; }

} // namespace ymcli::ui::screens
