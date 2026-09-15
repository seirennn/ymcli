#pragma once
#include <ftxui/component/component.hpp>
#include <vector>
#include <string>

#include "../../api/Models.hpp"
#include <functional>

namespace ymcli::ui::screens {

class HomeScreen {
public:
    using PlayTracksCallback = std::function<void(const std::vector<Track>& tracks, size_t index)>;
    using EnqueueCallback    = std::function<void(const Track& track)>;

    explicit HomeScreen(PlayTracksCallback play_cb = nullptr,
                        EnqueueCallback enqueue_cb = nullptr);
    ftxui::Component GetComponent();
    void SetRecentTracks(const std::vector<Track>& tracks);

private:
    std::vector<Track> recent_tracks_;
    int selected_ = 0;
    PlayTracksCallback play_cb_;
    EnqueueCallback enqueue_cb_;
    ftxui::Component component_;
};

} // namespace ymcli::ui::screens
