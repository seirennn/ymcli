#pragma once
#include <ftxui/component/component.hpp>
#include <string>
#include <vector>

#include "../../api/Models.hpp"

namespace ymcli::ui::screens {

struct ArtistSong { std::string title; };
struct ArtistAlbum { std::string title; std::string year; };

class ArtistScreen {
public:
    ArtistScreen();
    ftxui::Component GetComponent();
    
    void SetData(const std::string& name, const std::string& subs, const std::vector<ArtistSong>& songs, const std::vector<ArtistAlbum>& albums);
    void SetArtist(const Artist& artist) {
        std::vector<ArtistSong> songs;
        for (const auto& s : artist.top_songs) {
            songs.push_back({s.title});
        }
        std::vector<ArtistAlbum> albums;
        for (const auto& a : artist.albums) {
            albums.push_back({a.title, a.year});
        }
        SetData(artist.name, artist.subscriber_count, songs, albums);
    }

private:
    std::string name_, subs_;
    std::vector<ArtistSong> songs_;
    std::vector<ArtistAlbum> albums_;
    int selected_song_ = 0;
    int selected_album_ = 0;
    bool in_songs_ = true;
    ftxui::Component component_;
};

} // namespace ymcli::ui::screens
