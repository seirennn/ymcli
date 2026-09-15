#pragma once

#include "audio/Queue.hpp"
#include <string>
#include <functional>
#include <memory>

namespace ymcli {

class AudioEngine;
class InnerTube;
class Database;

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

enum class Screen {
    Home = 0,
    Search,
    Library,
    Queue,
    History,
    Favorites,
    Settings,
    AlbumDetail,
    ArtistDetail,
    PlaylistDetail,
};

class App {
public:
    App();
    ~App();

    App(const App&) = delete;
    App& operator=(const App&) = delete;

    void run();

    void playTrack(const Track& track);
    void playTracks(const std::vector<Track>& tracks, size_t start = 0);
    void addToQueue(const Track& track);
    void addToQueue(const std::vector<Track>& tracks);
    void togglePause();
    void nextTrack();
    void prevTrack();
    void seekRelative(double seconds);
    void setVolume(double vol);
    void toggleMute();
    void toggleShuffle();
    void cycleRepeat();

    bool authenticate(const std::string& cookie);
    bool autoDetectAuth();
    bool isAuthenticated() const;

    void toggleFavorite(const Track& track);
    bool isFavorite(const std::string& videoId);

    void search(const std::string& query);
    void navigateTo(Screen screen);
    void navigateToAlbum(const std::string& browseId);
    void navigateToArtist(const std::string& channelId);
    void navigateToPlaylist(const std::string& playlistId);

    const PlaybackState& playbackState() const { return playback_state_; }
    const SearchResults& searchResults() const { return search_results_; }
    const Album& currentAlbum() const { return current_album_; }
    const Artist& currentArtist() const { return current_artist_; }
    const Playlist& currentPlaylist() const { return current_playlist_; }
    ymcli::Queue& queue() { return queue_; }
    Database& database();

    int& activeScreen() { return active_screen_; }

private:
    void onTrackEnded();
    void playCurrentQueueTrack();

    std::unique_ptr<AudioEngine> audio_;
    std::unique_ptr<InnerTube> api_;
    std::unique_ptr<Database> db_;
    ymcli::Queue queue_;

    PlaybackState playback_state_;
    SearchResults search_results_;
    Album current_album_;
    Artist current_artist_;
    Playlist current_playlist_;

    int active_screen_ = 0;

    struct ScreenRef;
    std::unique_ptr<ScreenRef> screen_ref_;
};

} // namespace ymcli
