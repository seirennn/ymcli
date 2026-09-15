#pragma once

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include "PlaybackState.hpp"
#include <string>
#include <vector>

namespace ymcli::ui {

class FullscreenPlayer {
public:
    explicit FullscreenPlayer(const PlaybackState& state);

    ftxui::Component GetComponent();
    void Tick(); // Called periodically to advance ambient atmospheric phase

private:
    const PlaybackState& state_;
    double anim_phase_ = 0.0;

    ftxui::Element RenderVisualizer(int width, int height);
};

} // namespace ymcli::ui
