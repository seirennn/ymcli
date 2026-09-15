#pragma once
#include <ftxui/component/component.hpp>
#include <string>
#include <vector>

#include "../../api/Models.hpp"

namespace ymcli::ui::screens {

struct PlaylistTrack { std::string title; std::string artist; };

class PlaylistScreen {
public:
    PlaylistScreen();
    ftxui::Component GetComponent();
    
    void SetData(const std::string& title, const std::string& author, const std::string& count, const std::vector<PlaylistTrack>& tracks);
    void SetPlaylist(const Playlist& playlist) {
        std::vector<PlaylistTrack> tracks;
        for (const auto& t : playlist.tracks) {
            tracks.push_back({t.title, t.artist});
        }
        SetData(playlist.title, playlist.author, std::to_string(playlist.track_count), tracks);
    }

private:
    std::string title_, author_, count_;
    std::vector<PlaylistTrack> tracks_;
    int selected_ = 0;
    ftxui::Component component_;
};

} // namespace ymcli::ui::screens
