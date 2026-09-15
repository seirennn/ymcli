#pragma once

#include "Models.hpp"
#include <string>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>

namespace httplib {
    class Client;
}

namespace ymcli {

class InnerTube {
public:
    InnerTube();
    ~InnerTube();

    InnerTube(const InnerTube&) = delete;
    InnerTube& operator=(const InnerTube&) = delete;

    // Authentication via browser cookie string
    bool setAuthCookies(const std::string& cookie_string);
    bool loadAuthFile(const std::string& path);
    bool isAuthenticated() const;

    // API endpoints
    SearchResults search(const std::string& query, SearchFilter filter = SearchFilter::Songs);
    Album getAlbum(const std::string& browse_id);
    Artist getArtist(const std::string& channel_id);
    Playlist getPlaylist(const std::string& playlist_id);

    // Authenticated library endpoints
    std::vector<Track> getLikedSongs(int limit = 50);
    std::vector<Playlist> getUserPlaylists();
    std::vector<Track> getHistory();

private:
    std::string postRequest(const std::string& endpoint, const nlohmann::json& body);
    nlohmann::json buildContext() const;
    void applyAuthHeaders(void* headers_ptr);
    std::string computeSapisidHash(const std::string& sapisid, int64_t timestamp) const;

    void parseMusicShelf(const nlohmann::json& shelf, SearchResults& out_results);

    std::unique_ptr<httplib::Client> http_client_;
    std::string cookie_string_;
    std::string sapisid_;
    bool is_authenticated_ = false;
    mutable std::mutex http_mutex_;
};

} // namespace ymcli
