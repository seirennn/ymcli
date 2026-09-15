#pragma once

#include <ftxui/component/component.hpp>
#include <vector>
#include <functional>
#include "../../api/Models.hpp"

namespace ymcli::ui::screens {

class FavoritesScreen {
public:
    using PlayTracksCallback    = std::function<void(const std::vector<Track>& tracks, size_t index)>;
    using EnqueueCallback       = std::function<void(const Track& track)>;
    using AddToPlaylistCallback = std::function<void(const Track& track)>;
    using UnfavoriteCallback    = std::function<void(const Track& track)>;

    FavoritesScreen(PlayTracksCallback play_cb = nullptr,
                    EnqueueCallback enqueue_cb = nullptr,
                    AddToPlaylistCallback add_to_pl_cb = nullptr,
                    UnfavoriteCallback unfav_cb = nullptr);
    ftxui::Component GetComponent();

    void SetFavorites(const std::vector<Track>& favorites);

private:
    std::vector<Track> favorites_;
    int selected_ = 0;

    PlayTracksCallback play_cb_;
    EnqueueCallback enqueue_cb_;
    AddToPlaylistCallback add_to_pl_cb_;
    UnfavoriteCallback unfav_cb_;

    ftxui::Component component_;
};

} // namespace ymcli::ui::screens
