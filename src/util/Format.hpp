#pragma once

#include <string>

namespace Format {
    std::string formatDuration(int seconds);
    std::string formatDuration(double seconds);
    std::string truncate(const std::string& s, size_t maxLen);
    std::string toLower(const std::string& s);
    int parseDuration(const std::string& text);
}
