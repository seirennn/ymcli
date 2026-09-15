#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct Track {
    std::string video_id;
    std::string title;
    std::string artist;
    std::string album;
    std::string duration_text;
    int duration_seconds = 0;
    std::string thumbnail_url;
    bool is_explicit = false;
};

inline void from_json(const nlohmann::json& j, Track& t) {
    if (j.contains("video_id")) j.at("video_id").get_to(t.video_id);
    if (j.contains("title")) j.at("title").get_to(t.title);
    if (j.contains("artist")) j.at("artist").get_to(t.artist);
    if (j.contains("album")) j.at("album").get_to(t.album);
    if (j.contains("duration_text")) j.at("duration_text").get_to(t.duration_text);
    if (j.contains("duration_seconds")) j.at("duration_seconds").get_to(t.duration_seconds);
    if (j.contains("thumbnail_url")) j.at("thumbnail_url").get_to(t.thumbnail_url);
    if (j.contains("is_explicit")) j.at("is_explicit").get_to(t.is_explicit);
}

struct Album {
    std::string browse_id;
    std::string title;
    std::string artist;
    std::string year;
    std::string thumbnail_url;
    std::vector<Track> tracks;
};

inline void from_json(const nlohmann::json& j, Album& a) {
    if (j.contains("browse_id")) j.at("browse_id").get_to(a.browse_id);
    if (j.contains("title")) j.at("title").get_to(a.title);
    if (j.contains("artist")) j.at("artist").get_to(a.artist);
    if (j.contains("year")) j.at("year").get_to(a.year);
    if (j.contains("thumbnail_url")) j.at("thumbnail_url").get_to(a.thumbnail_url);
    if (j.contains("tracks")) j.at("tracks").get_to(a.tracks);
}

struct Artist {
    std::string channel_id;
    std::string name;
    std::string subscriber_count;
    std::string thumbnail_url;
    std::vector<Track> top_songs;
    std::vector<Album> albums;
};

inline void from_json(const nlohmann::json& j, Artist& a) {
    if (j.contains("channel_id")) j.at("channel_id").get_to(a.channel_id);
    if (j.contains("name")) j.at("name").get_to(a.name);
    if (j.contains("subscriber_count")) j.at("subscriber_count").get_to(a.subscriber_count);
    if (j.contains("thumbnail_url")) j.at("thumbnail_url").get_to(a.thumbnail_url);
    if (j.contains("top_songs")) j.at("top_songs").get_to(a.top_songs);
    if (j.contains("albums")) j.at("albums").get_to(a.albums);
}

struct Playlist {
    std::string playlist_id;
    std::string title;
    std::string author;
    int track_count = 0;
    std::string thumbnail_url;
    std::vector<Track> tracks;
};

inline void from_json(const nlohmann::json& j, Playlist& p) {
    if (j.contains("playlist_id")) j.at("playlist_id").get_to(p.playlist_id);
    if (j.contains("title")) j.at("title").get_to(p.title);
    if (j.contains("author")) j.at("author").get_to(p.author);
    if (j.contains("track_count")) j.at("track_count").get_to(p.track_count);
    if (j.contains("thumbnail_url")) j.at("thumbnail_url").get_to(p.thumbnail_url);
    if (j.contains("tracks")) j.at("tracks").get_to(p.tracks);
}

enum class SearchFilter { All, Songs, Albums, Artists, Playlists };

struct SearchResults {
    std::vector<Track> songs;
    std::vector<Album> albums;
    std::vector<Artist> artists;
    std::vector<Playlist> playlists;
};

inline void from_json(const nlohmann::json& j, SearchResults& s) {
    if (j.contains("songs")) j.at("songs").get_to(s.songs);
    if (j.contains("albums")) j.at("albums").get_to(s.albums);
    if (j.contains("artists")) j.at("artists").get_to(s.artists);
    if (j.contains("playlists")) j.at("playlists").get_to(s.playlists);
}
