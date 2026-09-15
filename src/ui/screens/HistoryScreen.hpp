#pragma once

#include <ftxui/component/component.hpp>
#include <vector>
#include <functional>
#include "../../api/Models.hpp"

namespace ymcli::ui::screens {

class HistoryScreen {
public:
    using PlayTrackCallback = std::function<void(const Track& track)>;

    HistoryScreen(PlayTrackCallback play_cb = nullptr);
    ftxui::Component GetComponent();

    void SetHistory(const std::vector<Track>& history);

private:
    std::vector<Track> history_;
    int selected_ = 0;

    PlayTrackCallback play_cb_;

    ftxui::Component component_;
};

} // namespace ymcli::ui::screens
