#pragma once

#include <ftxui/component/component.hpp>
#include <vector>
#include <functional>
#include "../../api/Models.hpp"

namespace ymcli::ui::screens {

class QueueScreen {
public:
    using JumpTrackCallback   = std::function<void(size_t index)>;
    using RemoveTrackCallback = std::function<void(size_t index)>;

    QueueScreen(JumpTrackCallback jump_cb = nullptr, RemoveTrackCallback remove_cb = nullptr);
    ftxui::Component GetComponent();

    void UpdateQueue(const std::vector<Track>& tracks, size_t current_index);

private:
    std::vector<Track> tracks_;
    size_t current_index_ = 0;
    int selected_ = 0;

    JumpTrackCallback jump_cb_;
    RemoveTrackCallback remove_cb_;

    ftxui::Component component_;
};

} // namespace ymcli::ui::screens
