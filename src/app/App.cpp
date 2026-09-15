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
        if (dur > 0.0) {
            playback_state_.duration = dur;
        }
        screen_ref_->screen.PostEvent(ftxui::Event::Custom);
    });

    audio_->onTitleChanged([this](const std::string& title) {
        if (!title.empty()) {
            playback_state_.title = title;
            screen_ref_->screen.PostEvent(ftxui::Event::Custom);
        }
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

bool App::autoDetectAuth(std::string* out_browser) {
    std::string cookies = CookieExtractor::autoExtractCookies(out_browser);
    if (!cookies.empty()) {
        return api_->setAuthCookies(cookies);
    }
    return false;
}

bool App::isAuthenticated() const {
    return api_->isAuthenticated();
}

void App::updateDataViews() {
    auto recent = db_->getPlayHistory(15);
    std::vector<ui::screens::RecentPlay> home_plays;
    for (const auto& t : recent) {
        home_plays.push_back({t.title, t.artist});
    }
    if (home_screen_ptr_) home_screen_ptr_->SetRecentPlays(home_plays);
    if (history_screen_ptr_) history_screen_ptr_->SetHistory(recent);
    if (favorites_screen_ptr_) favorites_screen_ptr_->SetFavorites(db_->getFavorites());
    if (queue_screen_ptr_) queue_screen_ptr_->UpdateQueue(queue_.tracks(), queue_.currentIndex());
    if (settings_screen_ptr_) settings_screen_ptr_->setAuthStatus(isAuthenticated());
}

void App::run() {
    ui::Sidebar sidebar(&active_screen_);
    ui::SearchBar search_bar([this](std::string query) {
        search(query);
    });

    ui::screens::HomeScreen home_screen;

    ui::screens::SearchScreen search_screen(
        [this](const Track& t) { playTrack(t); },
        [this](const Track& t) { addToQueue(t); },
        [this](const Track& t) { toggleFavorite(t); }
    );

    ui::screens::AlbumScreen album_screen;
    ui::screens::ArtistScreen artist_screen;
    ui::screens::PlaylistScreen playlist_screen;

    ui::screens::QueueScreen queue_screen(
        [this](size_t idx) { queue_.jumpTo(idx); playCurrentQueueTrack(); },
        [this](size_t idx) { queue_.removeTrack(idx); updateDataViews(); }
    );

    ui::screens::HistoryScreen history_screen(
        [this](const Track& t) { playTrack(t); }
    );

    ui::screens::FavoritesScreen favorites_screen(
        [this](const Track& t) { playTrack(t); },
        [this](const Track& t) { toggleFavorite(t); }
    );

    ui::screens::SettingsScreen settings_screen(
        [this](const std::string& cookie) { 
            bool ok = authenticate(cookie);
            updateDataViews();
            return ok;
        },
        [this](std::string& browser) { 
            bool ok = autoDetectAuth(&browser);
            updateDataViews();
            return ok;
        }
    );

    home_screen_ptr_ = &home_screen;
    search_screen_ptr_ = &search_screen;
    album_screen_ptr_ = &album_screen;
    artist_screen_ptr_ = &artist_screen;
    playlist_screen_ptr_ = &playlist_screen;
    queue_screen_ptr_ = &queue_screen;
    history_screen_ptr_ = &history_screen;
    favorites_screen_ptr_ = &favorites_screen;
    settings_screen_ptr_ = &settings_screen;

    updateDataViews();

    auto content_tab = ftxui::Container::Tab({
        home_screen.GetComponent(),
        search_screen.GetComponent(),
        favorites_screen.GetComponent(),
        queue_screen.GetComponent(),
        history_screen.GetComponent(),
        favorites_screen.GetComponent(),
        settings_screen.GetComponent(),
        album_screen.GetComponent(),
        artist_screen.GetComponent(),
        playlist_screen.GetComponent(),
    }, &active_screen_);

    ui::NowPlaying now_playing(reinterpret_cast<const ui::PlaybackState&>(playback_state_));

    ui::Layout layout(sidebar, search_bar, content_tab, now_playing);
    auto root = layout.GetComponent();

    root |= ftxui::CatchEvent([this, &search_bar, &content_tab, &search_screen](ftxui::Event event) {
        // If search input is focused, let it receive all normal keys without triggering media shortcuts
        if (search_bar.GetComponent()->Focused()) {
            if (event == ftxui::Event::Escape) {
                content_tab->TakeFocus();
                return true;
            }
            return false;
        }

        if (event == ftxui::Event::Character('q')) {
            screen_ref_->screen.ExitLoopClosure()();
            return true;
        }
        if (event == ftxui::Event::Character('/')) {
            search_bar.GetComponent()->TakeFocus();
            return true;
        }
        if (event == ftxui::Event::Character(' ')) {
            togglePause();
            return true;
        }
        if (event == ftxui::Event::Character('n')) {
            nextTrack();
            return true;
        }
        if (event == ftxui::Event::Character('p')) {
            prevTrack();
            return true;
        }
        if (event == ftxui::Event::Character('+') || event == ftxui::Event::Character('=')) {
            setVolume(playback_state_.volume + 5.0);
            return true;
        }
        if (event == ftxui::Event::Character('-')) {
            setVolume(playback_state_.volume - 5.0);
            return true;
        }
        if (event == ftxui::Event::Character('m')) {
            toggleMute();
            return true;
        }
        if (event == ftxui::Event::Character('s')) {
            toggleShuffle();
            return true;
        }
        if (event == ftxui::Event::Character('r')) {
            cycleRepeat();
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
        if (event == ftxui::Event::Character('1')) { active_screen_ = 0; content_tab->TakeFocus(); return true; }
        if (event == ftxui::Event::Character('2')) { active_screen_ = 1; search_screen.GetComponent()->TakeFocus(); return true; }
        if (event == ftxui::Event::Character('3')) { active_screen_ = 2; content_tab->TakeFocus(); return true; }
        if (event == ftxui::Event::Character('4')) { active_screen_ = 3; content_tab->TakeFocus(); return true; }
        if (event == ftxui::Event::Character('5')) { active_screen_ = 4; content_tab->TakeFocus(); return true; }
        if (event == ftxui::Event::Character('6')) { active_screen_ = 5; content_tab->TakeFocus(); return true; }
        if (event == ftxui::Event::Character('7')) { active_screen_ = 6; content_tab->TakeFocus(); return true; }

        return false;
    });

    screen_ref_->screen.Loop(root);
}

void App::playTrack(const Track& track) {
    playback_state_.title = track.title;
    playback_state_.artist = track.artist;
    playback_state_.album = track.album;
    playback_state_.duration = track.duration_seconds > 0 ? track.duration_seconds : 0.0;
    playback_state_.position = 0.0;
    playback_state_.has_track = true;
    playback_state_.is_paused = false;

    db_->addPlayHistory(track);
    updateDataViews();

    audio_->play(track.video_id);
    screen_ref_->screen.PostEvent(ftxui::Event::Custom);
}

void App::playTracks(const std::vector<Track>& tracks, size_t start) {
    queue_.playNow(tracks, start);
    playCurrentQueueTrack();
}

void App::addToQueue(const Track& track) {
    queue_.addTrack(track);
    updateDataViews();
    screen_ref_->screen.PostEvent(ftxui::Event::Custom);
}

void App::addToQueue(const std::vector<Track>& tracks) {
    queue_.addTracks(tracks);
    updateDataViews();
    screen_ref_->screen.PostEvent(ftxui::Event::Custom);
}

void App::togglePause() {
    audio_->togglePause();
    playback_state_.is_paused = !playback_state_.is_paused;
    screen_ref_->screen.PostEvent(ftxui::Event::Custom);
}

void App::nextTrack() {
    auto next = queue_.next();
    if (next) {
        playTrack(*next);
    } else {
        audio_->stop();
        playback_state_.has_track = false;
        playback_state_.is_paused = true;
        screen_ref_->screen.PostEvent(ftxui::Event::Custom);
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
    screen_ref_->screen.PostEvent(ftxui::Event::Custom);
}

void App::toggleMute() {
    audio_->toggleMute();
    playback_state_.is_muted = audio_->isMuted();
    screen_ref_->screen.PostEvent(ftxui::Event::Custom);
}

void App::toggleShuffle() {
    queue_.toggleShuffle();
    playback_state_.is_shuffled = queue_.isShuffled();
    screen_ref_->screen.PostEvent(ftxui::Event::Custom);
}

void App::cycleRepeat() {
    queue_.cycleRepeat();
    screen_ref_->screen.PostEvent(ftxui::Event::Custom);
}

void App::toggleFavorite(const Track& track) {
    if (db_->isFavorite(track.video_id)) {
        db_->removeFavorite(track.video_id);
    } else {
        db_->addFavorite(track);
    }
    updateDataViews();
    screen_ref_->screen.PostEvent(ftxui::Event::Custom);
}

bool App::isFavorite(const std::string& videoId) {
    return db_->isFavorite(videoId);
}

void App::search(const std::string& query) {
    if (query.empty()) return;
    db_->addSearchHistory(query);

    active_screen_ = static_cast<int>(Screen::Search);
    if (search_screen_ptr_) {
        search_screen_ptr_->SetLoading(true);
        search_screen_ptr_->GetComponent()->TakeFocus();
    }
    screen_ref_->screen.PostEvent(ftxui::Event::Custom);

    std::thread([this, query]() {
        auto results = api_->search(query);
        screen_ref_->screen.Post([this, results]() {
            search_results_ = results;
            if (search_screen_ptr_) {
                search_screen_ptr_->SetResults(results);
            }
        });
        screen_ref_->screen.PostEvent(ftxui::Event::Custom);
    }).detach();
}

void App::navigateTo(Screen screen) {
    active_screen_ = static_cast<int>(screen);
    screen_ref_->screen.PostEvent(ftxui::Event::Custom);
}

void App::navigateToAlbum(const std::string& browseId) {
    active_screen_ = static_cast<int>(Screen::AlbumDetail);
    screen_ref_->screen.PostEvent(ftxui::Event::Custom);
    std::thread([this, browseId]() {
        auto album = api_->getAlbum(browseId);
        screen_ref_->screen.Post([this, album]() {
            current_album_ = album;
            if (album_screen_ptr_) {
                album_screen_ptr_->SetAlbum(album);
            }
        });
        screen_ref_->screen.PostEvent(ftxui::Event::Custom);
    }).detach();
}

void App::navigateToArtist(const std::string& channelId) {
    active_screen_ = static_cast<int>(Screen::ArtistDetail);
    screen_ref_->screen.PostEvent(ftxui::Event::Custom);
    std::thread([this, channelId]() {
        auto artist = api_->getArtist(channelId);
        screen_ref_->screen.Post([this, artist]() {
            current_artist_ = artist;
            if (artist_screen_ptr_) {
                artist_screen_ptr_->SetArtist(artist);
            }
        });
        screen_ref_->screen.PostEvent(ftxui::Event::Custom);
    }).detach();
}

void App::navigateToPlaylist(const std::string& playlistId) {
    active_screen_ = static_cast<int>(Screen::PlaylistDetail);
    screen_ref_->screen.PostEvent(ftxui::Event::Custom);
    std::thread([this, playlistId]() {
        auto playlist = api_->getPlaylist(playlistId);
        screen_ref_->screen.Post([this, playlist]() {
            current_playlist_ = playlist;
            if (playlist_screen_ptr_) {
                playlist_screen_ptr_->SetPlaylist(playlist);
            }
        });
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
