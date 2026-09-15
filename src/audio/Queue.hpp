#pragma once

#include <vector>
#include <optional>
#include <random>
#include "../api/Models.hpp"

namespace ymcli {

enum class RepeatMode { Off, All, One };

class Queue {
public:
    Queue();

    // Queue manipulation
    void addTrack(const Track& track);
    void addTracks(const std::vector<Track>& tracks);
    void removeTrack(size_t index);
    void moveTrack(size_t from, size_t to);
    void clear();
    void clearExceptCurrent();

    // Navigation
    std::optional<Track> current() const;
    std::optional<Track> next();          // Advances and returns next track
    std::optional<Track> previous();      // Goes back and returns prev track
    void jumpTo(size_t index);

    // Modes
    void toggleShuffle();
    void cycleRepeat();                   // Off -> All -> One -> Off
    bool isShuffled() const;
    RepeatMode getRepeatMode() const;

    // State
    const std::vector<Track>& tracks() const;
    size_t currentIndex() const;
    size_t size() const;
    bool empty() const;

    // Play a list immediately (clears queue, adds tracks, starts from index 0)
    void playNow(const std::vector<Track>& tracks, size_t startIndex = 0);

private:
    std::vector<Track> tracks_;
    std::vector<size_t> shuffle_order_;
    int current_index_ = -1;
    bool shuffled_ = false;
    RepeatMode repeat_ = RepeatMode::Off;
    std::mt19937 rng_;

    size_t resolveIndex(size_t logical) const;
    void regenerateShuffle();
};

} // namespace ymcli
