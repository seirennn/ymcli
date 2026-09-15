#pragma once
#include <ftxui/component/component.hpp>
#include <vector>
#include <string>

namespace ymcli::ui::screens {

struct FavoriteItem { std::string title; std::string artist; };

class FavoritesScreen {
public:
    FavoritesScreen();
    ftxui::Component GetComponent();
    void SetFavorites(const std::vector<FavoriteItem>& items);

private:
    std::vector<FavoriteItem> items_;
    int selected_ = 0;
    ftxui::Component component_;
};

} // namespace ymcli::ui::screens
