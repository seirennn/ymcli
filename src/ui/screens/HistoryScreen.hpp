#pragma once

#include <ftxui/component/component.hpp>
#include <vector>
#include <functional>
#include "../../api/Models.hpp"

namespace ymcli::ui::screens {

class HistoryScreen {
public:
    using PlayTracksCallback    = std::function<void(const std::vector<Track>& tracks, size_t index)>;
    using EnqueueCallback       = std::function<void(const Track& track)>;
    using AddToPlaylistCallback = std::function<void(const Track& track)>;
    using FavoriteCallback      = std::function<void(const Track& track)>;

    HistoryScreen(PlayTracksCallback play_cb = nullptr,
                  EnqueueCallback enqueue_cb = nullptr,
                  AddToPlaylistCallback add_to_pl_cb = nullptr,
                  FavoriteCallback fav_cb = nullptr);
    ftxui::Component GetComponent();

    void SetHistory(const std::vector<Track>& history);

private:
    std::vector<Track> history_;
    int selected_ = 0;

    PlayTracksCallback play_cb_;
    EnqueueCallback enqueue_cb_;
    AddToPlaylistCallback add_to_pl_cb_;
    FavoriteCallback fav_cb_;

    ftxui::Component component_;
};

} // namespace ymcli::ui::screens
