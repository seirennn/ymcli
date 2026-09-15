#pragma once

#include <string>

namespace ymcli {

class CookieExtractor {
public:
    // Attempts automatic extraction of YouTube Music cookies from local browser profiles (Arc, Chrome, Brave, Firefox)
    static std::string autoExtractCookies();
};

} // namespace ymcli
