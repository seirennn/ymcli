#pragma once
#include <ftxui/component/component.hpp>
#include "PlaybackState.hpp"

namespace ymcli::ui {

class NowPlaying {
public:
    NowPlaying(const PlaybackState& state);
    ftxui::Component GetComponent();

private:
    const PlaybackState& state_;
};

} // namespace ymcli::ui
