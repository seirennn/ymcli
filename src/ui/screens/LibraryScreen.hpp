#pragma once

#include <ftxui/component/component.hpp>
#include <vector>
#include <string>
#include <functional>
#include "../../api/Models.hpp"

namespace ymcli::ui::screens {

class LibraryScreen {
public:
    using OpenPlaylistCallback   = std::function<void(const Playlist& playlist)>;
    using CreatePlaylistCallback = std::function<void(const std::string& name)>;
    using DeletePlaylistCallback = std::function<void(const std::string& playlistId)>;

    LibraryScreen(OpenPlaylistCallback open_cb = nullptr,
                  CreatePlaylistCallback create_cb = nullptr,
                  DeletePlaylistCallback delete_cb = nullptr);

    ftxui::Component GetComponent();

    void SetPlaylists(const std::vector<Playlist>& cloud_playlists,
                      const std::vector<Playlist>& local_playlists);

private:
    std::vector<Playlist> playlists_;
    int selected_ = 0;
    bool is_creating_ = false;
    std::string new_playlist_name_;

    OpenPlaylistCallback open_cb_;
    CreatePlaylistCallback create_cb_;
    DeletePlaylistCallback delete_cb_;

    ftxui::Component component_;
    ftxui::Component input_comp_;
};

} // namespace ymcli::ui::screens
