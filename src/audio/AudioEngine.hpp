#pragma once

#include <functional>
#include <string>
#include <memory>

namespace ymcli {

class AudioEngine {
public:
    using PositionCallback = std::function<void(double position, double duration)>;
    using TrackEndedCallback = std::function<void()>;
    using TitleCallback = std::function<void(const std::string& title)>;
    using StateCallback = std::function<void(bool is_paused)>;

    AudioEngine();
    ~AudioEngine();

    // Non-copyable, non-movable
    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;

    // Playback control
    void play(const std::string& videoId);  // Constructs YouTube URL and plays
    void playUrl(const std::string& url);   // Play arbitrary URL
    void pause();
    void resume();
    void togglePause();
    void stop();
    void seekRelative(double seconds);      // +/- seconds
    void seekAbsolute(double seconds);      // Absolute position

    // Volume
    void setVolume(double volume);          // 0-100
    double getVolume() const;
    void toggleMute();
    bool isMuted() const;

    // State queries
    bool isPlaying() const;
    bool isPaused() const;
    double getPosition() const;
    double getDuration() const;
    std::string getCurrentTitle() const;

    // Callbacks
    void onPositionChanged(PositionCallback cb);
    void onTrackEnded(TrackEndedCallback cb);
    void onTitleChanged(TitleCallback cb);
    void onStateChanged(StateCallback cb);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace ymcli
