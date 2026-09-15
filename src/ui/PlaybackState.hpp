#pragma once
#include <string>

namespace ymcli::ui {

enum class RepeatMode { Off, All, One };

struct PlaybackState {
    std::string title;
    std::string artist;
    std::string album;
    double position = 0.0;
    double duration = 0.0;
    double volume = 80.0;
    bool is_paused = true;
    bool is_muted = false;
    bool is_shuffled = false;
    RepeatMode repeat = RepeatMode::Off;
    bool has_track = false;
};

} // namespace ymcli::ui
