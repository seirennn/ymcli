#include "InnerTube.hpp"
#include "../util/Format.hpp"
#include "../util/Platform.hpp"

#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif
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

static void writeNetscapeCookies(const std::string& cookie_string, const std::string& file_path) {
    std::ofstream out(file_path);
    if (!out.is_open()) return;
    out << "# Netscape HTTP Cookie File\n";
    std::istringstream stream(cookie_string);
    std::string token;
    while (std::getline(stream, token, ';')) {
        size_t start = token.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        token = token.substr(start);
        size_t eq = token.find('=');
        if (eq == std::string::npos) continue;
        std::string name = token.substr(0, eq);
        std::string value = token.substr(eq + 1);
        out << ".youtube.com\tTRUE\t/\tTRUE\t2147483647\t" << name << "\t" << value << "\n";
    }
}

bool InnerTube::setAuthCookies(const std::string& cookie_string) {
    if (cookie_string.empty()) {
        is_authenticated_ = false;
        cookie_string_.clear();
        sapisid_.clear();
        return false;
    }

    // Sanitize cookie string to retain only valid printable ASCII characters
    std::string clean_cookies;
    clean_cookies.reserve(cookie_string.size());
    for (unsigned char c : cookie_string) {
        if (c >= 0x20 && c < 0x7F) {
            clean_cookies.push_back(static_cast<char>(c));
        }
    }

    if (clean_cookies.empty()) {
        is_authenticated_ = false;
        cookie_string_.clear();
        sapisid_.clear();
        return false;
    }

    cookie_string_ = std::move(clean_cookies);

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

    // Save auth string locally for persistence and create cookies.txt for mpv/yt-dlp
    if (is_authenticated_) {
        try {
            std::string auth_path = getConfigDir() + "/auth.json";
            std::string cookies_path = getConfigDir() + "/cookies.txt";
            ensureDirectory(getConfigDir());
            std::ofstream file(auth_path);
            if (file.is_open()) {
                json j = {{"cookie", cookie_string_}};
                file << j.dump(4, ' ', false, nlohmann::json::error_handler_t::replace);
            }
            writeNetscapeCookies(cookie_string_, cookies_path);
        } catch (...) {
            // Guard against any unexpected serialization or filesystem errors
        }
    }

    return is_authenticated_;
}

bool InnerTube::loadAuthFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    try {
        file.seekg(0, std::ios::end);
        auto len = file.tellg();
        if (len <= 0) return false;
        file.seekg(0, std::ios::beg);

        json j;
        file >> j;
        if (j.contains("cookie") && j["cookie"].is_string()) {
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

    std::string payload;
    try {
        payload = body.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace);
    } catch (...) {
        return "";
    }

    std::lock_guard<std::mutex> lock(http_mutex_);
    auto res = http_client_->Post(endpoint.c_str(), headers, payload, "application/json");

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

static void findItemRenderers(const json& j, const std::string& key, std::vector<json>& out) {
    if (j.is_object()) {
        for (auto it = j.begin(); it != j.end(); ++it) {
            if (it.key() == key) {
                out.push_back(it.value());
            } else {
                findItemRenderers(it.value(), key, out);
            }
        }
    } else if (j.is_array()) {
        for (const auto& elem : j) {
            findItemRenderers(elem, key, out);
        }
    }
}

static Track parseResponsiveTrack(const json& r) {
    Track track;
    try {
        if (r.contains("playlistItemData") && r["playlistItemData"].contains("videoId")) {
            track.video_id = r["playlistItemData"]["videoId"].get<std::string>();
        }

        if (r.contains("flexColumns")) {
            const auto& cols = r["flexColumns"];
            if (cols.size() > 0 && cols[0].contains("musicResponsiveListItemFlexColumnRenderer")) {
                const auto& text = cols[0]["musicResponsiveListItemFlexColumnRenderer"]["text"];
                if (text.contains("runs") && !text["runs"].empty()) {
                    track.title = text["runs"][0]["text"].get<std::string>();
                }
            }
            if (cols.size() > 1 && cols[1].contains("musicResponsiveListItemFlexColumnRenderer")) {
                const auto& text = cols[1]["musicResponsiveListItemFlexColumnRenderer"]["text"];
                if (text.contains("runs")) {
                    for (const auto& run : text["runs"]) {
                        std::string t = run["text"].get<std::string>();
                        if (t == " • " || t == " & " || t == ", ") continue;
                        if (track.artist.empty()) {
                            track.artist = t;
                        } else if (track.album.empty() && t != track.artist) {
                            track.album = t;
                        }
                    }
                }
            }
        }

        if (r.contains("fixedColumns") && !r["fixedColumns"].empty()) {
            const auto& fixed = r["fixedColumns"][0];
            if (fixed.contains("musicResponsiveListItemFixedColumnRenderer")) {
                const auto& text = fixed["musicResponsiveListItemFixedColumnRenderer"]["text"];
                if (text.contains("runs") && !text["runs"].empty()) {
                    track.duration_text = text["runs"][0]["text"].get<std::string>();
                    track.duration_seconds = parseDuration(track.duration_text);
                }
            }
        }
    } catch (...) {}
    return track;
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

        std::vector<json> headers;
        findItemRenderers(root, "musicDetailHeaderRenderer", headers);
        if (!headers.empty()) {
            const auto& h = headers[0];
            try { playlist.title = h["title"]["runs"][0]["text"].get<std::string>(); } catch (...) {}
            try { playlist.author = h["subtitle"]["runs"][0]["text"].get<std::string>(); } catch (...) {}
        }

        if (playlist.title.empty()) {
            std::vector<json> editable_headers;
            findItemRenderers(root, "musicEditablePlaylistDetailHeaderRenderer", editable_headers);
            if (!editable_headers.empty() && editable_headers[0].contains("header")) {
                const auto& eh = editable_headers[0]["header"];
                if (eh.contains("musicDetailHeaderRenderer")) {
                    const auto& h = eh["musicDetailHeaderRenderer"];
                    try { playlist.title = h["title"]["runs"][0]["text"].get<std::string>(); } catch (...) {}
                    try { playlist.author = h["subtitle"]["runs"][0]["text"].get<std::string>(); } catch (...) {}
                } else if (eh.contains("musicResponsiveHeaderRenderer")) {
                    const auto& h = eh["musicResponsiveHeaderRenderer"];
                    try { playlist.title = h["title"]["runs"][0]["text"].get<std::string>(); } catch (...) {}
                    try { playlist.author = h["subtitle"]["runs"][0]["text"].get<std::string>(); } catch (...) {}
                    if (playlist.author.empty() && h.contains("straplineTextOne")) {
                        try { playlist.author = h["straplineTextOne"]["runs"][0]["text"].get<std::string>(); } catch (...) {}
                    }
                }
            }
        }

        if (playlist.title.empty()) {
            std::vector<json> resp_headers;
            findItemRenderers(root, "musicResponsiveHeaderRenderer", resp_headers);
            if (!resp_headers.empty()) {
                const auto& h = resp_headers[0];
                try { playlist.title = h["title"]["runs"][0]["text"].get<std::string>(); } catch (...) {}
                try { playlist.author = h["subtitle"]["runs"][0]["text"].get<std::string>(); } catch (...) {}
                if (playlist.author.empty() && h.contains("straplineTextOne")) {
                    try { playlist.author = h["straplineTextOne"]["runs"][0]["text"].get<std::string>(); } catch (...) {}
                }
            }
        }

        std::vector<json> items;
        findItemRenderers(root, "musicResponsiveListItemRenderer", items);
        for (const auto& item : items) {
            Track t = parseResponsiveTrack(item);
            if (!t.video_id.empty()) {
                if (t.album.empty()) t.album = playlist.title;
                playlist.tracks.push_back(t);
            }
        }
        playlist.track_count = static_cast<int>(playlist.tracks.size());
    } catch (...) {}

    return playlist;
}

std::vector<Track> InnerTube::getLikedSongs(int limit) {
    auto playlist = getPlaylist("VLLM");
    return playlist.tracks;
}

std::vector<Playlist> InnerTube::getUserPlaylists() {
    std::vector<Playlist> playlists;
    if (!is_authenticated_) return playlists;

    json body = buildContext();
    body["browseId"] = "FEmusic_liked_playlists";

    std::string raw = postRequest("/youtubei/v1/browse?prettyPrint=false", body);
    if (raw.empty()) return playlists;

    try {
        json root = json::parse(raw);
        std::vector<json> items;
        findItemRenderers(root, "musicTwoRowItemRenderer", items);

        for (const auto& item : items) {
            Playlist p;
            try {
                if (item.contains("title") && item["title"].contains("runs") && !item["title"]["runs"].empty()) {
                    p.title = item["title"]["runs"][0]["text"].get<std::string>();
                }
                if (p.title == "New playlist" || p.title.empty()) continue;

                if (item.contains("navigationEndpoint") && 
                    item["navigationEndpoint"].contains("browseEndpoint") &&
                    item["navigationEndpoint"]["browseEndpoint"].contains("browseId")) {
                    p.playlist_id = item["navigationEndpoint"]["browseEndpoint"]["browseId"].get<std::string>();
                }
                if (p.playlist_id.empty()) continue;

                if (item.contains("subtitle") && item["subtitle"].contains("runs") && !item["subtitle"]["runs"].empty()) {
                    p.author = item["subtitle"]["runs"][0]["text"].get<std::string>();
                }

                playlists.push_back(p);
            } catch (...) {}
        }
    } catch (...) {}

    return playlists;
}

std::vector<Track> InnerTube::getHistory() {
    std::vector<Track> tracks;
    if (!is_authenticated_) return tracks;

    json body = buildContext();
    body["browseId"] = "FSMUSIC_HISTORY";

    std::string raw = postRequest("/youtubei/v1/browse?prettyPrint=false", body);
    if (raw.empty()) return tracks;

    try {
        json root = json::parse(raw);
        std::vector<json> items;
        findItemRenderers(root, "musicResponsiveListItemRenderer", items);
        for (const auto& item : items) {
            Track t = parseResponsiveTrack(item);
            if (!t.video_id.empty()) {
                tracks.push_back(t);
            }
        }
    } catch (...) {}

    return tracks;
}

} // namespace ymcli
