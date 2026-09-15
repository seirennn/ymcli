#include "App.hpp"
#include "audio/AudioEngine.hpp"
#include "api/InnerTube.hpp"
#include "api/CookieExtractor.hpp"
#include "db/Database.hpp"

#include "ui/Theme.hpp"
#include "ui/Sidebar.hpp"
#include "ui/SearchBar.hpp"
#include "ui/NowPlaying.hpp"
#include "ui/Layout.hpp"

#include "ui/screens/HomeScreen.hpp"
#include "ui/screens/SearchScreen.hpp"
#include "ui/screens/AlbumScreen.hpp"
#include "ui/screens/ArtistScreen.hpp"
#include "ui/screens/PlaylistScreen.hpp"
#include "ui/screens/QueueScreen.hpp"
#include "ui/screens/HistoryScreen.hpp"
#include "ui/screens/FavoritesScreen.hpp"
#include "ui/screens/SettingsScreen.hpp"

#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <thread>
#include <iostream>

namespace ymcli {

struct App::ScreenRef {
    ftxui::ScreenInteractive screen = ftxui::ScreenInteractive::Fullscreen();
};

App::App()
    : screen_ref_(std::make_unique<ScreenRef>())
{
    db_ = std::make_unique<Database>();
    api_ = std::make_unique<InnerTube>();
    audio_ = std::make_unique<AudioEngine>();

    audio_->onPositionChanged([this](double pos, double dur) {
        playback_state_.position = pos;
        playback_state_.duration = dur;
        screen_ref_->screen.PostEvent(ftxui::Event::Custom);
    });

    audio_->onTitleChanged([this](const std::string& title) {
        playback_state_.title = title;
        screen_ref_->screen.PostEvent(ftxui::Event::Custom);
    });

    audio_->onStateChanged([this](bool is_paused) {
        playback_state_.is_paused = is_paused;
        screen_ref_->screen.PostEvent(ftxui::Event::Custom);
    });

    audio_->onTrackEnded([this]() {
        onTrackEnded();
    });
}

App::~App() = default;

Database& App::database() {
    return *db_;
}

bool App::authenticate(const std::string& cookie) {
    return api_->setAuthCookies(cookie);
}

bool App::autoDetectAuth() {
    std::string cookies = CookieExtractor::autoExtractCookies();
    if (!cookies.empty()) {
        return api_->setAuthCookies(cookies);
    }
    return false;
}

bool App::isAuthenticated() const {
    return api_->isAuthenticated();
}

void App::run() {
    ui::Sidebar sidebar;
    ui::SearchBar search_bar([this](std::string query) {
        search(query);
    });

    ui::screens::HomeScreen home_screen;
    ui::screens::SearchScreen search_screen;
    ui::screens::AlbumScreen album_screen;
    ui::screens::ArtistScreen artist_screen;
    ui::screens::PlaylistScreen playlist_screen;
    ui::screens::QueueScreen queue_screen;
    ui::screens::HistoryScreen history_screen;
    ui::screens::FavoritesScreen favorites_screen;
    ui::screens::SettingsScreen settings_screen(
        [this](const std::string& cookie) { return authenticate(cookie); },
        [this]() { return autoDetectAuth(); }
    );

    settings_screen.setAuthStatus(isAuthenticated());

    auto recent = db_->getPlayHistory(10);
    std::vector<ui::screens::RecentPlay> home_plays;
    for (const auto& t : recent) {
        home_plays.push_back({t.title, t.artist});
    }
    home_screen.SetRecentPlays(home_plays);

    auto content_tab = ftxui::Container::Tab({
        home_screen.GetComponent(),
        search_screen.GetComponent(),
        home_screen.GetComponent(),
        queue_screen.GetComponent(),
        history_screen.GetComponent(),
        favorites_screen.GetComponent(),
        settings_screen.GetComponent(),
        album_screen.GetComponent(),
        artist_screen.GetComponent(),
        playlist_screen.GetComponent(),
    }, &active_screen_);

    auto sidebar_comp = sidebar.GetComponent();
    auto wrapped_sidebar = ftxui::CatchEvent(sidebar_comp, [this, &sidebar](ftxui::Event event) {
        if (event == ftxui::Event::Return) {
            active_screen_ = sidebar.selected();
            return true;
        }
        return false;
    });

    ui::PlaybackState ui_state;
    auto syncState = [this, &ui_state]() {
        ui_state.title = playback_state_.title;
        ui_state.artist = playback_state_.artist;
        ui_state.album = playback_state_.album;
        ui_state.position = playback_state_.position;
        ui_state.duration = playback_state_.duration;
        ui_state.volume = playback_state_.volume;
        ui_state.is_paused = playback_state_.is_paused;
        ui_state.is_muted = playback_state_.is_muted;
        ui_state.is_shuffled = playback_state_.is_shuffled;
        ui_state.has_track = playback_state_.has_track;
    };
    syncState();

    ui::NowPlaying now_playing(ui_state);

    ui::Layout layout(sidebar, search_bar, content_tab, now_playing);
    auto root = layout.GetComponent();

    root |= ftxui::CatchEvent([this, &search_screen, &syncState](ftxui::Event event) {
        if (event == ftxui::Event::Character('q')) {
            screen_ref_->screen.ExitLoopClosure()();
            return true;
        }
        if (event == ftxui::Event::Character(' ')) {
            togglePause();
            syncState();
            return true;
        }
        if (event == ftxui::Event::Character('n')) {
            nextTrack();
            syncState();
            return true;
        }
        if (event == ftxui::Event::Character('p')) {
            prevTrack();
            syncState();
            return true;
        }
        if (event == ftxui::Event::Character('+') || event == ftxui::Event::Character('=')) {
            setVolume(playback_state_.volume + 5.0);
            syncState();
            return true;
        }
        if (event == ftxui::Event::Character('-')) {
            setVolume(playback_state_.volume - 5.0);
            syncState();
            return true;
        }
        if (event == ftxui::Event::Character('m')) {
            toggleMute();
            syncState();
            return true;
        }
        if (event == ftxui::Event::Character('s')) {
            toggleShuffle();
            syncState();
            return true;
        }
        if (event == ftxui::Event::Character('r')) {
            cycleRepeat();
            syncState();
            return true;
        }
        if (event == ftxui::Event::Character('>')) {
            seekRelative(10.0);
            return true;
        }
        if (event == ftxui::Event::Character('<')) {
            seekRelative(-10.0);
            return true;
        }
        if (event == ftxui::Event::Character('1')) { active_screen_ = 0; return true; }
        if (event == ftxui::Event::Character('2')) { active_screen_ = 1; return true; }
        if (event == ftxui::Event::Character('3')) { active_screen_ = 2; return true; }
        if (event == ftxui::Event::Character('4')) { active_screen_ = 3; return true; }
        if (event == ftxui::Event::Character('5')) { active_screen_ = 4; return true; }
        if (event == ftxui::Event::Character('6')) { active_screen_ = 6; return true; }

        syncState();
        return false;
    });

    screen_ref_->screen.Loop(root);
}

void App::playTrack(const Track& track) {
    playback_state_.title = track.title;
    playback_state_.artist = track.artist;
    playback_state_.album = track.album;
    playback_state_.has_track = true;
    playback_state_.is_paused = false;

    db_->addPlayHistory(track);
    audio_->play(track.video_id);
}

void App::playTracks(const std::vector<Track>& tracks, size_t start) {
    queue_.playNow(tracks, start);
    playCurrentQueueTrack();
}

void App::addToQueue(const Track& track) {
    queue_.addTrack(track);
}

void App::addToQueue(const std::vector<Track>& tracks) {
    queue_.addTracks(tracks);
}

void App::togglePause() {
    audio_->togglePause();
    playback_state_.is_paused = !playback_state_.is_paused;
}

void App::nextTrack() {
    auto next = queue_.next();
    if (next) {
        playTrack(*next);
    } else {
        audio_->stop();
        playback_state_.has_track = false;
        playback_state_.is_paused = true;
    }
}

void App::prevTrack() {
    auto prev = queue_.previous();
    if (prev) {
        playTrack(*prev);
    }
}

void App::seekRelative(double seconds) {
    audio_->seekRelative(seconds);
}

void App::setVolume(double vol) {
    if (vol < 0.0) vol = 0.0;
    if (vol > 100.0) vol = 100.0;
    playback_state_.volume = vol;
    audio_->setVolume(vol);
}

void App::toggleMute() {
    audio_->toggleMute();
    playback_state_.is_muted = audio_->isMuted();
}

void App::toggleShuffle() {
    queue_.toggleShuffle();
    playback_state_.is_shuffled = queue_.isShuffled();
}

void App::cycleRepeat() {
    queue_.cycleRepeat();
}

void App::toggleFavorite(const Track& track) {
    if (db_->isFavorite(track.video_id)) {
        db_->removeFavorite(track.video_id);
    } else {
        db_->addFavorite(track);
    }
}

bool App::isFavorite(const std::string& videoId) {
    return db_->isFavorite(videoId);
}

void App::search(const std::string& query) {
    if (query.empty()) return;
    db_->addSearchHistory(query);

    active_screen_ = static_cast<int>(Screen::Search);

    std::thread([this, query]() {
        auto results = api_->search(query);
        search_results_ = results;
        screen_ref_->screen.PostEvent(ftxui::Event::Custom);
    }).detach();
}

void App::navigateTo(Screen screen) {
    active_screen_ = static_cast<int>(screen);
}

void App::navigateToAlbum(const std::string& browseId) {
    active_screen_ = static_cast<int>(Screen::AlbumDetail);
    std::thread([this, browseId]() {
        current_album_ = api_->getAlbum(browseId);
        screen_ref_->screen.PostEvent(ftxui::Event::Custom);
    }).detach();
}

void App::navigateToArtist(const std::string& channelId) {
    active_screen_ = static_cast<int>(Screen::ArtistDetail);
    std::thread([this, channelId]() {
        current_artist_ = api_->getArtist(channelId);
        screen_ref_->screen.PostEvent(ftxui::Event::Custom);
    }).detach();
}

void App::navigateToPlaylist(const std::string& playlistId) {
    active_screen_ = static_cast<int>(Screen::PlaylistDetail);
    std::thread([this, playlistId]() {
        current_playlist_ = api_->getPlaylist(playlistId);
        screen_ref_->screen.PostEvent(ftxui::Event::Custom);
    }).detach();
}

void App::onTrackEnded() {
    nextTrack();
}

void App::playCurrentQueueTrack() {
    auto current = queue_.current();
    if (current) {
        playTrack(*current);
    }
}

} // namespace ymcli
