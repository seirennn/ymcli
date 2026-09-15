#pragma once

#include <ftxui/component/component.hpp>
#include <string>
#include <vector>
#include <functional>
#include "../../api/Models.hpp"

namespace ymcli::ui::screens {

class PlaylistScreen {
public:
    using PlayTracksCallback    = std::function<void(const std::vector<Track>& tracks, size_t index)>;
    using EnqueueCallback       = std::function<void(const Track& track)>;
    using AddToPlaylistCallback = std::function<void(const Track& track)>;
    using FavoriteCallback      = std::function<void(const Track& track)>;
    using BackCallback          = std::function<void()>;

    PlaylistScreen(PlayTracksCallback play_cb = nullptr,
                   EnqueueCallback enqueue_cb = nullptr,
                   AddToPlaylistCallback add_to_pl_cb = nullptr,
                   FavoriteCallback fav_cb = nullptr,
                   BackCallback back_cb = nullptr);

    ftxui::Component GetComponent();
    
    void SetPlaylist(const Playlist& playlist);
    void SetLoading(bool is_loading);
    void SetHeader(const std::string& title, const std::string& author);

private:
    std::string title_;
    std::string author_;
    int track_count_ = 0;
    std::vector<Track> tracks_;
    int selected_ = 0;
    bool is_loading_ = false;

    PlayTracksCallback play_cb_;
    EnqueueCallback enqueue_cb_;
    AddToPlaylistCallback add_to_pl_cb_;
    FavoriteCallback fav_cb_;
    BackCallback back_cb_;

    ftxui::Component component_;
};

} // namespace ymcli::ui::screens
