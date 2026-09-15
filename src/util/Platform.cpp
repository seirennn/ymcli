#include "Platform.hpp"
#include <cstdlib>
#include <sys/stat.h>
#include <filesystem>

namespace ymcli {

std::string getDataDir() {
    const char* xdg_data = std::getenv("XDG_DATA_HOME");
    if (xdg_data && xdg_data[0] != '\0') {
        return std::string(xdg_data) + "/ymcli";
    }
    const char* home = std::getenv("HOME");
    if (home) {
        return std::string(home) + "/.local/share/ymcli";
    }
    return ".";
}

std::string getConfigDir() {
    const char* xdg_config = std::getenv("XDG_CONFIG_HOME");
    if (xdg_config && xdg_config[0] != '\0') {
        return std::string(xdg_config) + "/ymcli";
    }
    const char* home = std::getenv("HOME");
    if (home) {
        return std::string(home) + "/.config/ymcli";
    }
    return ".";
}

bool commandExists(const std::string& cmd) {
    std::string checkCmd = "command -v " + cmd + " > /dev/null 2>&1";
    int ret = std::system(checkCmd.c_str());
    return ret == 0;
}

void ensureDirectory(const std::string& path) {
    std::error_code ec;
    std::filesystem::create_directories(path, ec);
}

} // namespace ymcli
