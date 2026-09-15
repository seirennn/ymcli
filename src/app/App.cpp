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
#include "ui/screens/LibraryScreen.hpp"
#include "ui/screens/AlbumScreen.hpp"
#include "ui/screens/ArtistScreen.hpp"
#include "ui/screens/PlaylistScreen.hpp"
#include "ui/screens/QueueScreen.hpp"
#include "ui/screens/HistoryScreen.hpp"
#include "ui/screens/FavoritesScreen.hpp"
#include "ui/screens/SettingsScreen.hpp"

#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
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
        screen_ref_->screen.Post([this]() {
            onTrackEnded();
        });
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

void App::syncCloudData() {
    if (!isAuthenticated()) return;
    std::thread([this]() {
        auto playlists = api_->getUserPlaylists();
        auto liked = api_->getLikedSongs();
        screen_ref_->screen.Post([this, playlists, liked]() {
            cloud_playlists_ = playlists;
            cloud_liked_songs_ = liked;
            updateDataViews();
        });
    }).detach();
}

void App::updateDataViews() {
    auto recent = db_->getPlayHistory(15);
    std::vector<ui::screens::RecentPlay> home_plays;
    for (const auto& t : recent) {
        home_plays.push_back({t.title, t.artist});
    }
    if (home_screen_ptr_) home_screen_ptr_->SetRecentPlays(home_plays);
    if (history_screen_ptr_) history_screen_ptr_->SetHistory(recent);

    // Merge cloud liked songs with local favorites
    std::vector<Track> all_favs = db_->getFavorites();
    for (const auto& song : cloud_liked_songs_) {
        bool exists = false;
        for (const auto& f : all_favs) {
            if (f.video_id == song.video_id) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            all_favs.push_back(song);
        }
    }
    if (favorites_screen_ptr_) favorites_screen_ptr_->SetFavorites(all_favs);

    if (library_screen_ptr_) {
        library_screen_ptr_->SetPlaylists(cloud_playlists_, db_->getLocalPlaylists());
    }

    if (queue_screen_ptr_) queue_screen_ptr_->UpdateQueue(queue_.tracks(), queue_.currentIndex());
    if (settings_screen_ptr_) settings_screen_ptr_->setAuthStatus(isAuthenticated());
}

static ftxui::Element renderShortcutsModal() {
    auto key_style = [](const std::string& key) {
        return ftxui::text(key) | ftxui::bold | ftxui::color(ui::Theme::Accent) | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 13);
    };
    auto desc_style = [](const std::string& desc) {
        return ftxui::text(desc) | ftxui::color(ui::Theme::TextSecondary);
    };
    auto entry = [&](const std::string& key, const std::string& desc) {
        return ftxui::hbox({ key_style(key), desc_style(desc) });
    };

    auto playback_col = ftxui::vbox({
        ftxui::text("PLAYBACK CONTROLS") | ftxui::bold | ftxui::color(ui::Theme::Accent),
        ftxui::separator() | ftxui::color(ui::Theme::Border),
        entry("Space", "Play / Pause"),
        entry("n", "Next track (skip)"),
        entry("p", "Prev track / Restart"),
        entry("< / >", "Seek -10s / +10s"),
        entry("+ / -", "Volume up / down"),
        entry("m", "Toggle mute"),
        entry("s", "Toggle shuffle"),
        entry("r", "Cycle repeat mode")
    });

    auto nav_col = ftxui::vbox({
        ftxui::text("NAVIGATION") | ftxui::bold | ftxui::color(ui::Theme::Accent),
        ftxui::separator() | ftxui::color(ui::Theme::Border),
        entry("1 - 7", "Switch view tabs"),
        entry("/", "Search YouTube Music"),
        entry("j / k", "Navigate lists (Down / Up)"),
        entry("Tab", "Switch focus between panes"),
        entry("Esc", "Back / Unfocus"),
        entry("?", "Toggle this shortcuts guide"),
        entry("q", "Quit ymcli")
    });

    auto action_col = ftxui::vbox({
        ftxui::text("ACTIONS") | ftxui::bold | ftxui::color(ui::Theme::Accent),
        ftxui::separator() | ftxui::color(ui::Theme::Border),
        entry("Enter", "Play & queue remaining"),
        entry("a", "Add track to play queue"),
        entry("l / +", "Add track to playlist"),
        entry("f", "Save / Favorite track"),
        entry("P", "Play all tracks in playlist"),
        entry("A", "Add all tracks to play queue"),
        entry("d", "Remove track / Delete list")
    });

    return ftxui::vbox({
        ftxui::text("Y M C L I   S H O R T C U T S") | ftxui::bold | ftxui::color(ui::Theme::Accent) | ftxui::center,
        ftxui::separator() | ftxui::color(ui::Theme::Border),
        ftxui::hbox({
            playback_col | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 34),
            ftxui::separator() | ftxui::color(ui::Theme::Border),
            nav_col | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 36),
            ftxui::separator() | ftxui::color(ui::Theme::Border),
            action_col | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 34)
        }),
        ftxui::separator() | ftxui::color(ui::Theme::Border),
        ftxui::text("Press ? or Esc to return to player") | ftxui::color(ui::Theme::TextTertiary) | ftxui::center
    }) | ftxui::bgcolor(ui::Theme::Surface) | ftxui::borderRounded | ftxui::color(ui::Theme::FocusBorder);
}

void App::openAddToPlaylistModal(const Track& track) {
    modal_track_ = track;
    modal_local_playlists_ = db_->getLocalPlaylists();
    modal_playlist_sel_ = 0;
    modal_creating_new_ = false;
    modal_new_name_.clear();
    show_add_playlist_modal_ = true;
    screen_ref_->screen.PostEvent(ftxui::Event::Custom);
}

void App::run() {
    ui::Sidebar sidebar(&active_screen_);
    ui::SearchBar search_bar([this](std::string query) {
        search(query);
    });

    ui::screens::HomeScreen home_screen;

    ui::screens::SearchScreen search_screen(
        [this](const std::vector<Track>& t, size_t idx) { playTracks(t, idx); },
        [this](const Track& t) { addToQueue(t); },
        [this](const Track& t) { openAddToPlaylistModal(t); },
        [this](const Track& t) { toggleFavorite(t); }
    );

    ui::screens::LibraryScreen library_screen(
        [this](const Playlist& pl) {
            if (pl.playlist_id.rfind("local:", 0) == 0) {
                int id = std::stoi(pl.playlist_id.substr(6));
                navigateToLocalPlaylist(id, pl.title);
            } else {
                navigateToPlaylist(pl.playlist_id);
            }
        },
        [this](const std::string& name) {
            db_->createLocalPlaylist(name);
            updateDataViews();
        },
        [this](const std::string& plId) {
            if (plId.rfind("local:", 0) == 0) {
                int id = std::stoi(plId.substr(6));
                db_->deleteLocalPlaylist(id);
                updateDataViews();
            }
        }
    );

    ui::screens::AlbumScreen album_screen(
        [this](const std::vector<Track>& t, size_t idx) { playTracks(t, idx); },
        [this](const Track& t) { addToQueue(t); },
        [this](const Track& t) { openAddToPlaylistModal(t); },
        [this](const Track& t) { toggleFavorite(t); },
        [this]() { navigateTo(Screen::Search); }
    );

    ui::screens::ArtistScreen artist_screen(
        [this](const std::vector<Track>& t, size_t idx) { playTracks(t, idx); },
        [this](const std::string& bId) { navigateToAlbum(bId); },
        [this](const Track& t) { addToQueue(t); },
        [this](const Track& t) { openAddToPlaylistModal(t); },
        [this](const Track& t) { toggleFavorite(t); },
        [this]() { navigateTo(Screen::Search); }
    );

    ui::screens::PlaylistScreen playlist_screen(
        [this](const std::vector<Track>& t, size_t idx) { playTracks(t, idx); },
        [this](const Track& t) { addToQueue(t); },
        [this](const Track& t) { openAddToPlaylistModal(t); },
        [this](const Track& t) { toggleFavorite(t); },
        [this]() { navigateTo(Screen::Library); }
    );

    ui::screens::QueueScreen queue_screen(
        [this](size_t idx) { queue_.jumpTo(idx); playCurrentQueueTrack(); },
        [this](size_t idx) { queue_.removeTrack(idx); updateDataViews(); }
    );

    ui::screens::HistoryScreen history_screen(
        [this](const std::vector<Track>& t, size_t idx) { playTracks(t, idx); },
        [this](const Track& t) { addToQueue(t); },
        [this](const Track& t) { openAddToPlaylistModal(t); },
        [this](const Track& t) { toggleFavorite(t); }
    );

    ui::screens::FavoritesScreen favorites_screen(
        [this](const std::vector<Track>& t, size_t idx) { playTracks(t, idx); },
        [this](const Track& t) { addToQueue(t); },
        [this](const Track& t) { openAddToPlaylistModal(t); },
        [this](const Track& t) { toggleFavorite(t); }
    );

    ui::screens::SettingsScreen settings_screen(
        [this](const std::string& cookie) { 
            bool ok = authenticate(cookie);
            if (ok) syncCloudData();
            updateDataViews();
            return ok;
        },
        [this](std::string& browser) { 
            bool ok = autoDetectAuth(&browser);
            if (ok) syncCloudData();
            updateDataViews();
            return ok;
        }
    );

    home_screen_ptr_ = &home_screen;
    search_screen_ptr_ = &search_screen;
    library_screen_ptr_ = &library_screen;
    album_screen_ptr_ = &album_screen;
    artist_screen_ptr_ = &artist_screen;
    playlist_screen_ptr_ = &playlist_screen;
    queue_screen_ptr_ = &queue_screen;
    history_screen_ptr_ = &history_screen;
    favorites_screen_ptr_ = &favorites_screen;
    settings_screen_ptr_ = &settings_screen;

    updateDataViews();
    if (isAuthenticated()) {
        syncCloudData();
    }

    auto content_tab = ftxui::Container::Tab({
        home_screen.GetComponent(),        // 0: Home
        search_screen.GetComponent(),      // 1: Search
        library_screen.GetComponent(),     // 2: Library
        queue_screen.GetComponent(),       // 3: Queue
        history_screen.GetComponent(),     // 4: History
        favorites_screen.GetComponent(),   // 5: Favorites
        settings_screen.GetComponent(),    // 6: Settings
        album_screen.GetComponent(),       // 7: AlbumDetail
        artist_screen.GetComponent(),      // 8: ArtistDetail
        playlist_screen.GetComponent(),    // 9: PlaylistDetail
    }, &active_screen_);

    ui::NowPlaying now_playing(reinterpret_cast<const ui::PlaybackState&>(playback_state_));

    ui::Layout layout(sidebar, search_bar, content_tab, now_playing);
    auto base_layout = layout.GetComponent();

    ftxui::InputOption playlist_input_opt;
    playlist_input_opt.multiline = false;
    auto modal_input = ftxui::Input(&modal_new_name_, "New playlist name...", playlist_input_opt);

    auto root_container = ftxui::Container::Vertical({
        base_layout,
        modal_input
    });

    auto root = ftxui::Renderer(root_container, [this, base_layout, modal_input]() mutable -> ftxui::Element {
        auto main_element = base_layout->Render();

        if (show_shortcuts_modal_) {
            return ftxui::dbox({
                main_element,
                renderShortcutsModal() | ftxui::clear_under | ftxui::center
            });
        }

        if (show_add_playlist_modal_) {
            ftxui::Elements items;
            bool is_sel_new = (modal_playlist_sel_ == 0);
            items.push_back(
                ftxui::hbox({
                    ftxui::text(is_sel_new ? "› " : "  ") | ftxui::color(is_sel_new ? ui::Theme::Accent : ui::Theme::TextTertiary),
                    ftxui::text("[ + Create New Playlist ]") | ftxui::bold | ftxui::color(is_sel_new ? ui::Theme::Accent : ui::Theme::TextPrimary)
                }) | (is_sel_new ? ftxui::bgcolor(ui::Theme::Elevated) : ftxui::nothing)
            );

            for (size_t i = 0; i < modal_local_playlists_.size(); ++i) {
                bool is_sel = (modal_playlist_sel_ == static_cast<int>(i + 1));
                const auto& pl = modal_local_playlists_[i];
                items.push_back(
                    ftxui::hbox({
                        ftxui::text(is_sel ? "› " : "  ") | ftxui::color(is_sel ? ui::Theme::Accent : ui::Theme::TextTertiary),
                        ftxui::text(pl.title) | ftxui::color(is_sel ? ui::Theme::TextPrimary : ui::Theme::TextSecondary) | ftxui::flex,
                        ftxui::text(std::to_string(pl.track_count) + " tracks") | ftxui::color(ui::Theme::TextTertiary)
                    }) | (is_sel ? ftxui::bgcolor(ui::Theme::Elevated) : ftxui::nothing)
                );
            }

            ftxui::Element body;
            if (modal_creating_new_) {
                body = ftxui::vbox({
                    ftxui::text("Playlist Name:") | ftxui::color(ui::Theme::TextSecondary),
                    modal_input->Render() | ftxui::bgcolor(ui::Theme::Elevated),
                    ftxui::separator() | ftxui::color(ui::Theme::Border),
                    ftxui::text("[Enter] Confirm   [Esc] Back") | ftxui::color(ui::Theme::TextTertiary) | ftxui::center
                });
            } else {
                body = ftxui::vbox({
                    ftxui::vbox(std::move(items)) | ftxui::yframe | ftxui::size(ftxui::HEIGHT, ftxui::LESS_THAN, 10),
                    ftxui::separator() | ftxui::color(ui::Theme::Border),
                    ftxui::text("[Enter] Select   [Esc] Cancel") | ftxui::color(ui::Theme::TextTertiary) | ftxui::center
                });
            }

            auto modal_box = ftxui::vbox({
                ftxui::text("ADD TO PLAYLIST") | ftxui::bold | ftxui::color(ui::Theme::Accent) | ftxui::center,
                ftxui::text(modal_track_.title.empty() ? "" : (modal_track_.title + " — " + modal_track_.artist)) | ftxui::color(ui::Theme::TextTertiary) | ftxui::center,
                ftxui::separator() | ftxui::color(ui::Theme::Border),
                body
            }) | ftxui::bgcolor(ui::Theme::Surface) | ftxui::borderRounded | ftxui::color(ui::Theme::FocusBorder)
               | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 50);

            return ftxui::dbox({
                main_element,
                modal_box | ftxui::clear_under | ftxui::center
            });
        }

        return main_element;
    });

    root |= ftxui::CatchEvent([this, &search_bar, &content_tab, &search_screen, modal_input](ftxui::Event event) {
        if (show_shortcuts_modal_) {
            if (event == ftxui::Event::Character('?') || 
                event == ftxui::Event::Escape || 
                event == ftxui::Event::Return ||
                event == ftxui::Event::Character('q')) {
                show_shortcuts_modal_ = false;
                return true;
            }
            return true;
        }

        if (show_add_playlist_modal_) {
            if (modal_creating_new_) {
                if (event == ftxui::Event::Escape) {
                    modal_creating_new_ = false;
                    modal_new_name_.clear();
                    return true;
                }
                if (event == ftxui::Event::Return) {
                    if (!modal_new_name_.empty()) {
                        int pid = db_->createLocalPlaylist(modal_new_name_);
                        if (pid > 0 && !modal_track_.video_id.empty()) {
                            db_->addTrackToLocalPlaylist(pid, modal_track_);
                        }
                    }
                    show_add_playlist_modal_ = false;
                    modal_creating_new_ = false;
                    modal_new_name_.clear();
                    updateDataViews();
                    return true;
                }
                return modal_input->OnEvent(event);
            }

            if (event == ftxui::Event::Escape) {
                show_add_playlist_modal_ = false;
                return true;
            }
            if (event == ftxui::Event::Character('j') || event == ftxui::Event::ArrowDown) {
                modal_playlist_sel_ = std::min(static_cast<int>(modal_local_playlists_.size()), modal_playlist_sel_ + 1);
                return true;
            }
            if (event == ftxui::Event::Character('k') || event == ftxui::Event::ArrowUp) {
                modal_playlist_sel_ = std::max(0, modal_playlist_sel_ - 1);
                return true;
            }
            if (event == ftxui::Event::Return) {
                if (modal_playlist_sel_ == 0) {
                    modal_creating_new_ = true;
                    modal_new_name_.clear();
                    modal_input->TakeFocus();
                    return true;
                } else {
                    int idx = modal_playlist_sel_ - 1;
                    if (idx >= 0 && idx < static_cast<int>(modal_local_playlists_.size())) {
                        std::string plId = modal_local_playlists_[idx].playlist_id;
                        if (plId.rfind("local:", 0) == 0) {
                            int pid = std::stoi(plId.substr(6));
                            db_->addTrackToLocalPlaylist(pid, modal_track_);
                        }
                    }
                    show_add_playlist_modal_ = false;
                    updateDataViews();
                    return true;
                }
            }
            return true;
        }

        // If search input is focused, let it receive all normal keys without triggering media shortcuts
        if (search_bar.GetComponent()->Focused()) {
            if (event == ftxui::Event::Escape) {
                content_tab->TakeFocus();
                return true;
            }
            return false;
        }

        if (event == ftxui::Event::Character('?')) {
            show_shortcuts_modal_ = true;
            return true;
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
        if (event == ftxui::Event::Character('>') || event == ftxui::Event::ArrowRight) {
            seekRelative(10.0);
            return true;
        }
        if (event == ftxui::Event::Character('<') || event == ftxui::Event::ArrowLeft) {
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
    if (tracks.empty()) return;
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
    if (playback_state_.position > 3.0) {
        seekRelative(-playback_state_.position);
        return;
    }
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

void App::navigateToLocalPlaylist(int playlist_id, const std::string& title) {
    active_screen_ = static_cast<int>(Screen::PlaylistDetail);
    Playlist pl;
    pl.playlist_id = "local:" + std::to_string(playlist_id);
    pl.title = title;
    pl.author = "Local Playlist";
    pl.tracks = db_->getLocalPlaylistTracks(playlist_id);
    pl.track_count = static_cast<int>(pl.tracks.size());
    current_playlist_ = pl;
    if (playlist_screen_ptr_) {
        playlist_screen_ptr_->SetPlaylist(pl);
    }
    screen_ref_->screen.PostEvent(ftxui::Event::Custom);
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
