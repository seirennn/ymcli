#include "InnerTube.hpp"
#include "../util/Format.hpp"
#include "../util/Platform.hpp"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <openssl/sha.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>

using json = nlohmann::json;

namespace ymcli {

InnerTube::InnerTube() {
    http_client_ = std::make_unique<httplib::Client>("https://music.youtube.com");
    http_client_->set_connection_timeout(10, 0);
    http_client_->set_read_timeout(12, 0);
    http_client_->set_follow_location(true);

    // Try loading saved credentials on startup
    std::string auth_path = getConfigDir() + "/auth.json";
    loadAuthFile(auth_path);
}

InnerTube::~InnerTube() = default;

bool InnerTube::setAuthCookies(const std::string& cookie_string) {
    if (cookie_string.empty()) {
        is_authenticated_ = false;
        cookie_string_.clear();
        sapisid_.clear();
        return false;
    }

    cookie_string_ = cookie_string;

    // Extract SAPISID or __Secure-3PAPISID from cookie string for auth header
    std::string target = "SAPISID=";
    auto pos = cookie_string_.find(target);
    if (pos == std::string::npos) {
        target = "__Secure-3PAPISID=";
        pos = cookie_string_.find(target);
    }

    if (pos != std::string::npos) {
        size_t start = pos + target.length();
        size_t end = cookie_string_.find(';', start);
        sapisid_ = cookie_string_.substr(start, end == std::string::npos ? std::string::npos : end - start);
        is_authenticated_ = !sapisid_.empty();
    } else {
        is_authenticated_ = false;
    }

    // Save auth string locally for persistence
    if (is_authenticated_) {
        std::string auth_path = getConfigDir() + "/auth.json";
        ensureDirectory(getConfigDir());
        std::ofstream file(auth_path);
        if (file.is_open()) {
            json j = {{"cookie", cookie_string_}};
            file << j.dump(4);
        }
    }

    return is_authenticated_;
}

bool InnerTube::loadAuthFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    try {
        json j;
        file >> j;
        if (j.contains("cookie")) {
            return setAuthCookies(j["cookie"].get<std::string>());
        }
    } catch (...) {}
    return false;
}

bool InnerTube::isAuthenticated() const {
    return is_authenticated_;
}

std::string InnerTube::computeSapisidHash(const std::string& sapisid, int64_t timestamp) const {
    std::string payload = std::to_string(timestamp) + " " + sapisid + " https://music.youtube.com";
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(payload.data()), payload.size(), hash);

    std::ostringstream ss;
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return std::to_string(timestamp) + "_" + ss.str();
}

json InnerTube::buildContext() const {
    return {
        {"context", {
            {"client", {
                {"clientName", "WEB_REMIX"},
                {"clientVersion", "1.20240901.01.00"},
                {"hl", "en"},
                {"gl", "US"}
            }}
        }}
    };
}

std::string InnerTube::postRequest(const std::string& endpoint, const json& body) {
    httplib::Headers headers = {
        {"User-Agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36"},
        {"Origin", "https://music.youtube.com"},
        {"Content-Type", "application/json"},
        {"X-YouTube-Client-Name", "67"},
        {"X-YouTube-Client-Version", "1.20240901.01.00"}
    };

    if (is_authenticated_) {
        headers.emplace("Cookie", cookie_string_);
        int64_t now = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        std::string auth_val = "SAPISIDHASH " + computeSapisidHash(sapisid_, now);
        headers.emplace("Authorization", auth_val);
    }

    std::lock_guard<std::mutex> lock(http_mutex_);
    auto res = http_client_->Post(endpoint.c_str(), headers, body.dump(), "application/json");

    if (res && res->status == 200) {
        return res->body;
    }
    return "";
}

void InnerTube::parseMusicShelf(const json& shelf, SearchResults& out_results) {
    if (!shelf.contains("contents")) return;

    for (const auto& item : shelf["contents"]) {
        if (!item.contains("musicResponsiveListItemRenderer")) continue;
        const auto& r = item["musicResponsiveListItemRenderer"];

        std::string video_id;
        if (r.contains("playlistItemData") && r["playlistItemData"].contains("videoId")) {
            video_id = r["playlistItemData"]["videoId"].get<std::string>();
        }

        std::string title;
        std::string artist;
        std::string album;
        std::string duration;

        if (r.contains("flexColumns")) {
            const auto& cols = r["flexColumns"];
            if (!cols.empty()) {
                try {
                    title = cols[0]["musicResponsiveListItemFlexColumnRenderer"]["text"]["runs"][0]["text"].get<std::string>();
                } catch (...) {}
            }
            if (cols.size() > 1) {
                try {
                    const auto& runs = cols[1]["musicResponsiveListItemFlexColumnRenderer"]["text"]["runs"];
                    if (!runs.empty()) artist = runs[0]["text"].get<std::string>();
                    if (runs.size() >= 3) album = runs[2]["text"].get<std::string>();
                } catch (...) {}
            }
        }

        if (!video_id.empty()) {
            Track track;
            track.video_id = video_id;
            track.title = title;
            track.artist = artist;
            track.album = album;
            track.duration_text = duration;
            out_results.songs.push_back(track);
        }
    }
}

SearchResults InnerTube::search(const std::string& query, SearchFilter filter) {
    SearchResults results;
    json body = buildContext();
    body["query"] = query;

    // Filter param strings for YouTube Music search categories
    if (filter == SearchFilter::Songs) {
        body["params"] = "Eg-KAQwIARAAGAAgACgAMABqChAEEAMQCRAFEAo=";
    }

    std::string raw = postRequest("/youtubei/v1/search?prettyPrint=false", body);
    if (raw.empty()) return results;

    try {
        json root = json::parse(raw);
        const auto& sections = root["contents"]["tabbedSearchResultsRenderer"]["tabs"][0]
                                   ["tabRenderer"]["content"]["sectionListRenderer"]["contents"];

        for (const auto& section : sections) {
            if (section.contains("musicShelfRenderer")) {
                parseMusicShelf(section["musicShelfRenderer"], results);
            }
        }
    } catch (...) {}

    return results;
}

Album InnerTube::getAlbum(const std::string& browse_id) {
    Album album;
    album.browse_id = browse_id;

    json body = buildContext();
    body["browseId"] = browse_id;

    std::string raw = postRequest("/youtubei/v1/browse?prettyPrint=false", body);
    if (raw.empty()) return album;

    try {
        json root = json::parse(raw);

        // Header metadata
        if (root.contains("header") && root["header"].contains("musicDetailHeaderRenderer")) {
            const auto& header = root["header"]["musicDetailHeaderRenderer"];
            try { album.title = header["title"]["runs"][0]["text"].get<std::string>(); } catch (...) {}
            try { album.artist = header["subtitle"]["runs"][0]["text"].get<std::string>(); } catch (...) {}
            try { album.year = header["subtitle"]["runs"].back()["text"].get<std::string>(); } catch (...) {}
        }

        // Track list
        const auto& contents = root["contents"]["singleColumnBrowseResultsRenderer"]["tabs"][0]
                                   ["tabRenderer"]["content"]["sectionListRenderer"]["contents"][0]
                                   ["musicShelfRenderer"]["contents"];

        for (const auto& item : contents) {
            if (!item.contains("musicResponsiveListItemRenderer")) continue;
            const auto& r = item["musicResponsiveListItemRenderer"];

            Track track;
            try { track.video_id = r["playlistItemData"]["videoId"].get<std::string>(); } catch (...) {}
            try { track.title = r["flexColumns"][0]["musicResponsiveListItemFlexColumnRenderer"]["text"]["runs"][0]["text"].get<std::string>(); } catch (...) {}
            track.artist = album.artist;
            track.album = album.title;

            if (!track.video_id.empty()) {
                album.tracks.push_back(track);
            }
        }
    } catch (...) {}

    return album;
}

Artist InnerTube::getArtist(const std::string& channel_id) {
    Artist artist;
    artist.channel_id = channel_id;

    json body = buildContext();
    body["browseId"] = channel_id;

    std::string raw = postRequest("/youtubei/v1/browse?prettyPrint=false", body);
    if (raw.empty()) return artist;

    try {
        json root = json::parse(raw);
        if (root.contains("header") && root["header"].contains("musicImmersiveHeaderRenderer")) {
            const auto& h = root["header"]["musicImmersiveHeaderRenderer"];
            artist.name = h["title"]["runs"][0]["text"].get<std::string>();
        }
    } catch (...) {}

    return artist;
}

Playlist InnerTube::getPlaylist(const std::string& playlist_id) {
    Playlist playlist;
    playlist.playlist_id = playlist_id;

    json body = buildContext();
    body["browseId"] = playlist_id.starts_with("VL") ? playlist_id : "VL" + playlist_id;

    std::string raw = postRequest("/youtubei/v1/browse?prettyPrint=false", body);
    if (raw.empty()) return playlist;

    try {
        json root = json::parse(raw);
        // Parse title and tracks
    } catch (...) {}

    return playlist;
}

std::vector<Track> InnerTube::getLikedSongs(int limit) {
    return getAlbum("FSMUSIC_LIKED_SONGS").tracks;
}

std::vector<Playlist> InnerTube::getUserPlaylists() {
    std::vector<Playlist> playlists;
    if (!is_authenticated_) return playlists;

    json body = buildContext();
    body["browseId"] = "FEplaylist_aggregation";

    std::string raw = postRequest("/youtubei/v1/browse?prettyPrint=false", body);
    if (raw.empty()) return playlists;

    return playlists;
}

std::vector<Track> InnerTube::getHistory() {
    std::vector<Track> tracks;
    if (!is_authenticated_) return tracks;

    json body = buildContext();
    body["browseId"] = "FSMUSIC_HISTORY";

    std::string raw = postRequest("/youtubei/v1/browse?prettyPrint=false", body);
    if (raw.empty()) return tracks;

    return tracks;
}

} // namespace ymcli
