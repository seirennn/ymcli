#include "Database.hpp"
#include "../util/Platform.hpp"
#include <sqlite3.h>
#include <iostream>
#include <ctime>

namespace ymcli {

Database::Database() {
    std::string dataDir = getDataDir();
    ensureDirectory(dataDir);
    std::string dbPath = dataDir + "/ymcli.db";
    
    if (sqlite3_open(dbPath.c_str(), &db_) != SQLITE_OK) {
        std::cerr << "Failed to open database: " << dbPath << "\n";
    } else {
        initTables();
    }
}

Database::~Database() {
    if (db_) {
        sqlite3_close(db_);
    }
}

void Database::execute(const std::string& sql) {
    char* errMsg = nullptr;
    if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << "\n";
        sqlite3_free(errMsg);
    }
}

void Database::initTables() {
    execute(R"(
        CREATE TABLE IF NOT EXISTS search_history (
            id INTEGER PRIMARY KEY,
            query TEXT,
            timestamp INTEGER
        );
        CREATE TABLE IF NOT EXISTS play_history (
            id INTEGER PRIMARY KEY,
            video_id TEXT,
            title TEXT,
            artist TEXT,
            album TEXT,
            played_at INTEGER
        );
        CREATE TABLE IF NOT EXISTS favorites (
            id INTEGER PRIMARY KEY,
            video_id TEXT UNIQUE,
            title TEXT,
            artist TEXT,
            album TEXT,
            added_at INTEGER
        );
        CREATE TABLE IF NOT EXISTS local_playlists (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT UNIQUE,
            created_at INTEGER
        );
        CREATE TABLE IF NOT EXISTS local_playlist_tracks (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            playlist_id INTEGER,
            video_id TEXT,
            title TEXT,
            artist TEXT,
            album TEXT,
            duration_text TEXT,
            duration_seconds INTEGER,
            added_at INTEGER
        );
    )");
}

void Database::addSearchHistory(const std::string& query) {
    const char* sql = "INSERT INTO search_history (query, timestamp) VALUES (?, ?)";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, query.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int64(stmt, 2, std::time(nullptr));
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

std::vector<std::string> Database::getSearchHistory(int limit) {
    std::vector<std::string> results;
    const char* sql = "SELECT query FROM search_history ORDER BY timestamp DESC LIMIT ?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, limit);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            results.emplace_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        }
        sqlite3_finalize(stmt);
    }
    return results;
}

void Database::addPlayHistory(const Track& track) {
    const char* sql = "INSERT INTO play_history (video_id, title, artist, album, played_at) VALUES (?, ?, ?, ?, ?)";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, track.video_id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, track.title.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, track.artist.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, track.album.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 5, std::time(nullptr));
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

std::vector<Track> Database::getPlayHistory(int limit) {
    std::vector<Track> results;
    const char* sql = "SELECT video_id, title, artist, album FROM play_history ORDER BY played_at DESC LIMIT ?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, limit);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Track t;
            t.video_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            t.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            t.artist = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            if (auto album = sqlite3_column_text(stmt, 3)) t.album = reinterpret_cast<const char*>(album);
            results.push_back(t);
        }
        sqlite3_finalize(stmt);
    }
    return results;
}

void Database::addFavorite(const Track& track) {
    const char* sql = "INSERT OR REPLACE INTO favorites (video_id, title, artist, album, added_at) VALUES (?, ?, ?, ?, ?)";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, track.video_id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, track.title.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, track.artist.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, track.album.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 5, std::time(nullptr));
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

void Database::removeFavorite(const std::string& videoId) {
    const char* sql = "DELETE FROM favorites WHERE video_id = ?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, videoId.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

bool Database::isFavorite(const std::string& videoId) {
    const char* sql = "SELECT 1 FROM favorites WHERE video_id = ?";
    sqlite3_stmt* stmt;
    bool exists = false;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, videoId.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            exists = true;
        }
        sqlite3_finalize(stmt);
    }
    return exists;
}

std::vector<Track> Database::getFavorites() {
    std::vector<Track> results;
    const char* sql = "SELECT video_id, title, artist, album FROM favorites ORDER BY added_at DESC";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Track t;
            t.video_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            t.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            t.artist = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            if (auto album = sqlite3_column_text(stmt, 3)) t.album = reinterpret_cast<const char*>(album);
            results.push_back(t);
        }
        sqlite3_finalize(stmt);
    }
    return results;
}

std::vector<Playlist> Database::getLocalPlaylists() {
    std::vector<Playlist> list;
    const char* sql = "SELECT p.id, p.name, COUNT(t.id) "
                      "FROM local_playlists p "
                      "LEFT JOIN local_playlist_tracks t ON p.id = t.playlist_id "
                      "GROUP BY p.id ORDER BY p.name ASC";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Playlist pl;
            int id = sqlite3_column_int(stmt, 0);
            pl.playlist_id = "local:" + std::to_string(id);
            pl.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            pl.author = "Local Playlist";
            pl.track_count = sqlite3_column_int(stmt, 2);
            list.push_back(pl);
        }
        sqlite3_finalize(stmt);
    }
    return list;
}

int Database::createLocalPlaylist(const std::string& name) {
    if (name.empty()) return -1;
    const char* sql = "INSERT OR IGNORE INTO local_playlists (name, created_at) VALUES (?, ?)";
    sqlite3_stmt* stmt;
    int new_id = -1;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 2, std::time(nullptr));
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            new_id = static_cast<int>(sqlite3_last_insert_rowid(db_));
        }
        sqlite3_finalize(stmt);
    }
    return new_id;
}

bool Database::addTrackToLocalPlaylist(int playlist_id, const Track& track) {
    const char* sql = "INSERT INTO local_playlist_tracks (playlist_id, video_id, title, artist, album, duration_text, duration_seconds, added_at) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?)";
    sqlite3_stmt* stmt;
    bool ok = false;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, playlist_id);
        sqlite3_bind_text(stmt, 2, track.video_id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, track.title.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, track.artist.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, track.album.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 6, track.duration_text.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 7, track.duration_seconds);
        sqlite3_bind_int64(stmt, 8, std::time(nullptr));
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            ok = true;
        }
        sqlite3_finalize(stmt);
    }
    return ok;
}

std::vector<Track> Database::getLocalPlaylistTracks(int playlist_id) {
    std::vector<Track> tracks;
    const char* sql = "SELECT video_id, title, artist, album, duration_text, duration_seconds "
                      "FROM local_playlist_tracks WHERE playlist_id = ? ORDER BY id ASC";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, playlist_id);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Track t;
            t.video_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            t.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            t.artist = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            if (auto alb = sqlite3_column_text(stmt, 3)) t.album = reinterpret_cast<const char*>(alb);
            if (auto dur = sqlite3_column_text(stmt, 4)) t.duration_text = reinterpret_cast<const char*>(dur);
            t.duration_seconds = sqlite3_column_int(stmt, 5);
            tracks.push_back(t);
        }
        sqlite3_finalize(stmt);
    }
    return tracks;
}

bool Database::deleteLocalPlaylist(int playlist_id) {
    const char* sql1 = "DELETE FROM local_playlist_tracks WHERE playlist_id = ?";
    const char* sql2 = "DELETE FROM local_playlists WHERE id = ?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql1, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, playlist_id);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    if (sqlite3_prepare_v2(db_, sql2, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, playlist_id);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    return true;
}

} // namespace ymcli
