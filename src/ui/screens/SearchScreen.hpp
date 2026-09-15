#pragma once

#include <ftxui/component/component.hpp>
#include <vector>
#include <string>
#include <functional>
#include "../../api/Models.hpp"

namespace ymcli::ui::screens {

class SearchScreen {
public:
    using PlayTrackCallback = std::function<void(const Track& track)>;
    using EnqueueCallback   = std::function<void(const Track& track)>;
    using FavoriteCallback  = std::function<void(const Track& track)>;

    SearchScreen(PlayTrackCallback play_cb = nullptr,
                 EnqueueCallback enqueue_cb = nullptr,
                 FavoriteCallback favorite_cb = nullptr);

    ftxui::Component GetComponent();

    void SetLoading(bool loading);
    void SetResults(const SearchResults& results);

private:
    bool is_loading_ = false;
    int tab_index_ = 0;
    std::vector<std::string> tab_names_ = {" Songs ", " Albums ", " Artists ", " Playlists "};
    ftxui::Component tab_toggle_;

    SearchResults results_;
    int selected_item_ = 0;

    PlayTrackCallback play_cb_;
    EnqueueCallback enqueue_cb_;
    FavoriteCallback favorite_cb_;

    ftxui::Component component_;
};

} // namespace ymcli::ui::screens
