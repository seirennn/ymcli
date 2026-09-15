#include "Format.hpp"
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace Format {

std::string formatDuration(int seconds) {
    int m = seconds / 60;
    int s = seconds % 60;
    std::ostringstream oss;
    oss << m << ":" << std::setw(2) << std::setfill('0') << s;
    return oss.str();
}

std::string formatDuration(double seconds) {
    return formatDuration(static_cast<int>(seconds));
}

std::string truncate(const std::string& s, size_t maxLen) {
    if (s.length() <= maxLen) return s;
    if (maxLen <= 3) return s.substr(0, maxLen);
    return s.substr(0, maxLen - 3) + "...";
}

std::string toLower(const std::string& s) {
    std::string res = s;
    std::transform(res.begin(), res.end(), res.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return res;
}

int parseDuration(const std::string& text) {
    int total_seconds = 0;
    std::istringstream iss(text);
    std::string token;
    std::vector<int> parts;
    
    while (std::getline(iss, token, ':')) {
        try {
            parts.push_back(std::stoi(token));
        } catch (...) {
            return 0;
        }
    }
    
    if (parts.size() == 2) {
        total_seconds = parts[0] * 60 + parts[1];
    } else if (parts.size() == 3) {
        total_seconds = parts[0] * 3600 + parts[1] * 60 + parts[2];
    }
    
    return total_seconds;
}

} // namespace Format
