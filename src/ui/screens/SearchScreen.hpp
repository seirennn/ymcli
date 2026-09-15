#pragma once

#include <ftxui/component/component.hpp>
#include <vector>
#include <string>
#include <functional>
#include "../../api/Models.hpp"

namespace ymcli::ui::screens {

class SearchScreen {
public:
    using PlayTracksCallback    = std::function<void(const std::vector<Track>& tracks, size_t index)>;
    using EnqueueCallback       = std::function<void(const Track& track)>;
    using AddToPlaylistCallback = std::function<void(const Track& track)>;
    using FavoriteCallback      = std::function<void(const Track& track)>;

    SearchScreen(PlayTracksCallback play_cb = nullptr,
                 EnqueueCallback enqueue_cb = nullptr,
                 AddToPlaylistCallback add_to_pl_cb = nullptr,
                 FavoriteCallback favorite_cb = nullptr);

    ftxui::Component GetComponent();

    void SetLoading(bool loading);
    void SetResults(const SearchResults& results);

private:
    bool is_loading_ = false;
    int tab_index_ = 0;
    std::vector<std::string> tab_names_ = {"[songs]", "[albums]", "[artists]", "[playlists]"};
    ftxui::Component tab_toggle_;

    SearchResults results_;
    int selected_item_ = 0;

    PlayTracksCallback play_cb_;
    EnqueueCallback enqueue_cb_;
    AddToPlaylistCallback add_to_pl_cb_;
    FavoriteCallback favorite_cb_;

    ftxui::Component component_;
};

} // namespace ymcli::ui::screens
