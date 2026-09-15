#include "app/App.hpp"
#include "util/Platform.hpp"
#include <iostream>
#include <cstdlib>

int main(int argc, char** argv) {
    // Check runtime dependencies
    bool mpv_ok = ymcli::commandExists("mpv");
    bool ytdlp_ok = ymcli::commandExists("yt-dlp");

    if (!mpv_ok || !ytdlp_ok) {
        std::cerr << "========================================================\n";
        std::cerr << " ymcli — Missing Runtime Dependencies                   \n";
        std::cerr << "========================================================\n\n";
        if (!mpv_ok) {
            std::cerr << " [!] mpv command is missing.\n";
            std::cerr << "     Install via Homebrew: brew install mpv\n\n";
        }
        if (!ytdlp_ok) {
            std::cerr << " [!] yt-dlp command is missing.\n";
            std::cerr << "     Install via Homebrew: brew install yt-dlp\n\n";
        }
        std::cerr << "Please install the missing dependencies and try again.\n";
        return EXIT_FAILURE;
    }

    try {
        ymcli::App app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
