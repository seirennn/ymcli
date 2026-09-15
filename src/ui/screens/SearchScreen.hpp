#pragma once
#include <ftxui/component/component.hpp>
#include <vector>
#include <string>

namespace ymcli::ui::screens {

struct SearchResultSong { std::string title; std::string artist; std::string duration; };
struct SearchResultAlbum { std::string title; std::string artist; std::string year; };
struct SearchResultArtist { std::string name; std::string subscribers; };
struct SearchResultPlaylist { std::string title; std::string author; std::string track_count; };

class SearchScreen {
public:
    SearchScreen();
    ftxui::Component GetComponent();
    
    void SetLoading(bool loading);
    void SetSongs(const std::vector<SearchResultSong>& songs);
    void SetAlbums(const std::vector<SearchResultAlbum>& albums);
    void SetArtists(const std::vector<SearchResultArtist>& artists);
    void SetPlaylists(const std::vector<SearchResultPlaylist>& playlists);

private:
    bool is_loading_ = false;
    int tab_index_ = 0;
    std::vector<std::string> tab_names_ = {"Songs", "Albums", "Artists", "Playlists"};
    ftxui::Component tab_toggle_;

    std::vector<SearchResultSong> songs_;
    std::vector<SearchResultAlbum> albums_;
    std::vector<SearchResultArtist> artists_;
    std::vector<SearchResultPlaylist> playlists_;

    int selected_item_ = 0;
    ftxui::Component component_;
};

} // namespace ymcli::ui::screens
