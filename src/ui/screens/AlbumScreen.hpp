#pragma once
#include <ftxui/component/component.hpp>
#include <string>
#include <vector>

#include "../../api/Models.hpp"

namespace ymcli::ui::screens {

struct AlbumTrack { std::string title; std::string duration; };

class AlbumScreen {
public:
    AlbumScreen();
    ftxui::Component GetComponent();
    
    void SetData(const std::string& title, const std::string& artist, const std::string& year, const std::vector<AlbumTrack>& tracks);
    void SetAlbum(const Album& album) {
        std::vector<AlbumTrack> tracks;
        for (const auto& t : album.tracks) {
            tracks.push_back({t.title, t.duration_text});
        }
        SetData(album.title, album.artist, album.year, tracks);
    }

private:
    std::string title_, artist_, year_;
    std::vector<AlbumTrack> tracks_;
    int selected_ = 0;
    ftxui::Component component_;
};

} // namespace ymcli::ui::screens
