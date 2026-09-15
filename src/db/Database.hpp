#pragma once

#include <string>
#include <vector>
#include "../api/Models.hpp"

struct sqlite3;

namespace ymcli {

class Database {
public:
    Database();
    ~Database();

    // Disable copy/move
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    void addSearchHistory(const std::string& query);
    std::vector<std::string> getSearchHistory(int limit = 20);

    void addPlayHistory(const Track& track);
    std::vector<Track> getPlayHistory(int limit = 50);

    void addFavorite(const Track& track);
    void removeFavorite(const std::string& videoId);
    bool isFavorite(const std::string& videoId);
    std::vector<Track> getFavorites();

    // Local playlists
    std::vector<Playlist> getLocalPlaylists();
    int createLocalPlaylist(const std::string& name);
    bool addTrackToLocalPlaylist(int playlist_id, const Track& track);
    std::vector<Track> getLocalPlaylistTracks(int playlist_id);
    bool deleteLocalPlaylist(int playlist_id);

private:
    void initTables();
    void execute(const std::string& sql);

    sqlite3* db_ = nullptr;
};

} // namespace ymcli
