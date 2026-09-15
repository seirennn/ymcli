#pragma once
#include <ftxui/component/component.hpp>
#include <string>
#include <vector>

namespace ymcli::ui::screens {

struct ArtistSong { std::string title; };
struct ArtistAlbum { std::string title; std::string year; };

class ArtistScreen {
public:
    ArtistScreen();
    ftxui::Component GetComponent();
    
    void SetData(const std::string& name, const std::string& subs, const std::vector<ArtistSong>& songs, const std::vector<ArtistAlbum>& albums);

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
