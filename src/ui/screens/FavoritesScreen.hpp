#pragma once

#include <ftxui/component/component.hpp>
#include <vector>
#include <functional>
#include "../../api/Models.hpp"

namespace ymcli::ui::screens {

class FavoritesScreen {
public:
    using PlayTrackCallback    = std::function<void(const Track& track)>;
    using UnfavoriteCallback   = std::function<void(const Track& track)>;

    FavoritesScreen(PlayTrackCallback play_cb = nullptr, UnfavoriteCallback unfav_cb = nullptr);
    ftxui::Component GetComponent();

    void SetFavorites(const std::vector<Track>& favorites);

private:
    std::vector<Track> favorites_;
    int selected_ = 0;

    PlayTrackCallback play_cb_;
    UnfavoriteCallback unfav_cb_;

    ftxui::Component component_;
};

} // namespace ymcli::ui::screens
