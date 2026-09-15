#include "AudioEngine.hpp"
#include "../util/Platform.hpp"
#include <mpv/client.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <stdexcept>
#include <iostream>
#include <filesystem>

namespace ymcli {

struct AudioEngine::Impl {
    mpv_handle* mpv = nullptr;
    std::thread event_thread;
    std::atomic<bool> running{false};

    // Callbacks
    PositionCallback position_cb;
    TrackEndedCallback track_ended_cb;
    TitleCallback title_cb;
    StateCallback state_cb;
    AudioLevelCallback audio_level_cb;
    mutable std::mutex cb_mutex;

    Impl() {
        mpv = mpv_create();
        if (!mpv) {
            throw std::runtime_error("Failed to create mpv context");
        }

        mpv_set_option_string(mpv, "vo", "null");
        mpv_set_option_string(mpv, "vid", "no");
        mpv_set_option_string(mpv, "audio-display", "no");
        mpv_set_option_string(mpv, "force-window", "no");
        mpv_set_option_string(mpv, "ytdl", "yes");
        mpv_set_option_string(mpv, "ytdl-format", "bestaudio/best");
        mpv_set_option_string(mpv, "network-timeout", "15");
        mpv_set_option_string(mpv, "demuxer-max-bytes", "20971520");

        std::string cookie_path = getConfigDir() + "/cookies.txt";
        std::error_code ec;
        if (std::filesystem::exists(cookie_path, ec)) {
            std::string raw_opt = "cookies=" + cookie_path;
            mpv_set_option_string(mpv, "ytdl-raw-options", raw_opt.c_str());
        }

        if (mpv_initialize(mpv) < 0) {
            throw std::runtime_error("Failed to initialize mpv");
        }

        mpv_observe_property(mpv, 1, "time-pos", MPV_FORMAT_DOUBLE);
        mpv_observe_property(mpv, 2, "duration", MPV_FORMAT_DOUBLE);
        mpv_observe_property(mpv, 3, "media-title", MPV_FORMAT_STRING);
        mpv_observe_property(mpv, 4, "pause", MPV_FORMAT_FLAG);

        running = true;
        event_thread = std::thread([this]() { event_loop(); });
    }

    ~Impl() {
        running = false;
        if (mpv) {
            mpv_wakeup(mpv);
        }
        if (event_thread.joinable()) {
            event_thread.join();
        }
        if (mpv) {
            mpv_terminate_destroy(mpv);
        }
    }

    void event_loop() {
        while (running) {
            mpv_event* event = mpv_wait_event(mpv, 0.1);
            if (!event || event->event_id == MPV_EVENT_NONE) continue;
            if (event->event_id == MPV_EVENT_SHUTDOWN) break;

            if (event->event_id == MPV_EVENT_END_FILE) {
                mpv_event_end_file* end_file = (mpv_event_end_file*)event->data;
                if (end_file->reason == MPV_END_FILE_REASON_EOF || 
                    end_file->reason == MPV_END_FILE_REASON_ERROR) {
                    std::lock_guard<std::mutex> lock(cb_mutex);
                    if (track_ended_cb) {
                        track_ended_cb();
                    }
                }
            } else if (event->event_id == MPV_EVENT_PROPERTY_CHANGE) {
                mpv_event_property* prop = (mpv_event_property*)event->data;
                if (!prop || !prop->data) continue;

                std::lock_guard<std::mutex> lock(cb_mutex);
                
                if (event->reply_userdata == 1 && prop->format == MPV_FORMAT_DOUBLE) {
                    double pos = *(double*)prop->data;
                    double dur = 0.0;
                    mpv_get_property(mpv, "duration", MPV_FORMAT_DOUBLE, &dur);
                    if (position_cb) position_cb(pos, dur);
                } else if (event->reply_userdata == 2 && prop->format == MPV_FORMAT_DOUBLE) {
                    double dur = *(double*)prop->data;
                    double pos = 0.0;
                    mpv_get_property(mpv, "time-pos", MPV_FORMAT_DOUBLE, &pos);
                    if (position_cb) position_cb(pos, dur);
                } else if (event->reply_userdata == 3 && prop->format == MPV_FORMAT_STRING) {
                    const char* str = *(char**)prop->data;
                    if (str && title_cb) title_cb(std::string(str));
                } else if (event->reply_userdata == 4 && prop->format == MPV_FORMAT_FLAG) {
                    bool is_paused = *(int*)prop->data != 0;
                    if (state_cb) state_cb(is_paused);
                }
            }
        }
    }
};

AudioEngine::AudioEngine() : impl_(std::make_unique<Impl>()) {}
AudioEngine::~AudioEngine() = default;

void AudioEngine::play(const std::string& videoId) {
    std::string cookie_path = getConfigDir() + "/cookies.txt";
    std::error_code ec;
    if (std::filesystem::exists(cookie_path, ec)) {
        std::string raw_opt = "cookies=" + cookie_path;
        mpv_set_option_string(impl_->mpv, "ytdl-raw-options", raw_opt.c_str());
    }
    playUrl("https://www.youtube.com/watch?v=" + videoId);
}

void AudioEngine::playUrl(const std::string& url) {
    const char* cmd[] = {"loadfile", url.c_str(), "replace", nullptr};
    mpv_command(impl_->mpv, cmd);
}

void AudioEngine::pause() {
    int pause_val = 1;
    mpv_set_property(impl_->mpv, "pause", MPV_FORMAT_FLAG, &pause_val);
}

void AudioEngine::resume() {
    int pause_val = 0;
    mpv_set_property(impl_->mpv, "pause", MPV_FORMAT_FLAG, &pause_val);
}

void AudioEngine::togglePause() {
    if (isPaused()) {
        resume();
    } else {
        pause();
    }
}

void AudioEngine::stop() {
    const char* cmd[] = {"stop", nullptr};
    mpv_command(impl_->mpv, cmd);
}

void AudioEngine::seekRelative(double seconds) {
    std::string sec_str = std::to_string(seconds);
    const char* cmd[] = {"seek", sec_str.c_str(), "relative", nullptr};
    mpv_command(impl_->mpv, cmd);
}

void AudioEngine::seekAbsolute(double seconds) {
    std::string sec_str = std::to_string(seconds);
    const char* cmd[] = {"seek", sec_str.c_str(), "absolute", nullptr};
    mpv_command(impl_->mpv, cmd);
}

void AudioEngine::setVolume(double volume) {
    mpv_set_property(impl_->mpv, "volume", MPV_FORMAT_DOUBLE, &volume);
}

double AudioEngine::getVolume() const {
    double vol = 0.0;
    mpv_get_property(impl_->mpv, "volume", MPV_FORMAT_DOUBLE, &vol);
    return vol;
}

void AudioEngine::toggleMute() {
    int mute_val = 0;
    mpv_get_property(impl_->mpv, "mute", MPV_FORMAT_FLAG, &mute_val);
    mute_val = !mute_val;
    mpv_set_property(impl_->mpv, "mute", MPV_FORMAT_FLAG, &mute_val);
}

bool AudioEngine::isMuted() const {
    int mute_val = 0;
    mpv_get_property(impl_->mpv, "mute", MPV_FORMAT_FLAG, &mute_val);
    return mute_val != 0;
}

bool AudioEngine::isPlaying() const {
    int idle_active = 0;
    mpv_get_property(impl_->mpv, "idle-active", MPV_FORMAT_FLAG, &idle_active);
    return !idle_active && !isPaused();
}

bool AudioEngine::isPaused() const {
    int pause_val = 0;
    mpv_get_property(impl_->mpv, "pause", MPV_FORMAT_FLAG, &pause_val);
    return pause_val != 0;
}

double AudioEngine::getPosition() const {
    double pos = 0.0;
    mpv_get_property(impl_->mpv, "time-pos", MPV_FORMAT_DOUBLE, &pos);
    return pos;
}

double AudioEngine::getDuration() const {
    double dur = 0.0;
    mpv_get_property(impl_->mpv, "duration", MPV_FORMAT_DOUBLE, &dur);
    return dur;
}

std::string AudioEngine::getCurrentTitle() const {
    char* title = nullptr;
    mpv_get_property(impl_->mpv, "media-title", MPV_FORMAT_STRING, &title);
    std::string res;
    if (title) {
        res = title;
        mpv_free(title);
    }
    return res;
}

void AudioEngine::onPositionChanged(PositionCallback cb) {
    std::lock_guard<std::mutex> lock(impl_->cb_mutex);
    impl_->position_cb = std::move(cb);
}

void AudioEngine::onTrackEnded(TrackEndedCallback cb) {
    std::lock_guard<std::mutex> lock(impl_->cb_mutex);
    impl_->track_ended_cb = std::move(cb);
}

void AudioEngine::onTitleChanged(TitleCallback cb) {
    std::lock_guard<std::mutex> lock(impl_->cb_mutex);
    impl_->title_cb = std::move(cb);
}

void AudioEngine::onStateChanged(StateCallback cb) {
    std::lock_guard<std::mutex> lock(impl_->cb_mutex);
    impl_->state_cb = std::move(cb);
}

void AudioEngine::onAudioLevel(AudioLevelCallback cb) {
    std::lock_guard<std::mutex> lock(impl_->cb_mutex);
    impl_->audio_level_cb = std::move(cb);
}

double AudioEngine::getAudioLevel() const {
    if (isPaused() || isMuted() || !isPlaying()) return 0.0;
    return getVolume() / 100.0;
}

} // namespace ymcli
