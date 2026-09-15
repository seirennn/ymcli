#pragma once

#include <string>

namespace ymcli {

class CookieExtractor {
public:
    // Attempts automatic extraction of YouTube Music cookies from local browser profiles (Firefox, Arc, Chrome, Brave, Edge, Zen, etc.)
    static std::string autoExtractCookies(std::string* out_browser = nullptr);
};

} // namespace ymcli
