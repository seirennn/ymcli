#pragma once

#include <string>

namespace ymcli {

inline std::string formatDuration(int total_seconds) {
    if (total_seconds < 0) return "0:00";
    int minutes = total_seconds / 60;
    int seconds = total_seconds % 60;
    return std::to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + std::to_string(seconds);
}

inline std::string formatDuration(double seconds) {
    return formatDuration(static_cast<int>(seconds));
}

inline std::string truncate(const std::string& s, size_t max_len) {
    if (s.size() <= max_len) return s;
    if (max_len <= 3) return s.substr(0, max_len);
    return s.substr(0, max_len - 3) + "...";
}

inline std::string toLower(const std::string& s) {
    std::string result = s;
    for (auto& c : result) {
        if (c >= 'A' && c <= 'Z') c += 32;
    }
    return result;
}

inline int parseDuration(const std::string& text) {
    int total = 0;
    int current = 0;
    for (char c : text) {
        if (c == ':') {
            total = total * 60 + current;
            current = 0;
        } else if (c >= '0' && c <= '9') {
            current = current * 10 + (c - '0');
        }
    }
    return total * 60 + current;
}

} // namespace ymcli
