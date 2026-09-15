#pragma once

#include <ftxui/component/component.hpp>
#include <string>
#include <vector>
#include <functional>
#include "../../api/Models.hpp"

namespace ymcli::ui::screens {

class ArtistScreen {
public:
    using PlayTracksCallback    = std::function<void(const std::vector<Track>& tracks, size_t index)>;
    using OpenAlbumCallback     = std::function<void(const std::string& browseId)>;
    using EnqueueCallback       = std::function<void(const Track& track)>;
    using AddToPlaylistCallback = std::function<void(const Track& track)>;
    using FavoriteCallback      = std::function<void(const Track& track)>;
    using BackCallback          = std::function<void()>;

    ArtistScreen(PlayTracksCallback play_cb = nullptr,
                 OpenAlbumCallback album_cb = nullptr,
                 EnqueueCallback enqueue_cb = nullptr,
                 AddToPlaylistCallback add_to_pl_cb = nullptr,
                 FavoriteCallback fav_cb = nullptr,
                 BackCallback back_cb = nullptr);

    ftxui::Component GetComponent();
    void SetArtist(const Artist& artist);

private:
    std::string name_;
    std::string subs_;
    std::vector<Track> songs_;
    std::vector<Album> albums_;
    int selected_song_ = 0;
    int selected_album_ = 0;
    bool in_songs_ = true;

    PlayTracksCallback play_cb_;
    OpenAlbumCallback album_cb_;
    EnqueueCallback enqueue_cb_;
    AddToPlaylistCallback add_to_pl_cb_;
    FavoriteCallback fav_cb_;
    BackCallback back_cb_;

    ftxui::Component component_;
};

} // namespace ymcli::ui::screens
