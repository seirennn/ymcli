#include "app/App.hpp"
#include "util/Platform.hpp"
#include <iostream>
#include <cstdlib>

int main(int argc, char** argv) {
    // Check runtime dependencies
    bool mpv_ok = ymcli::commandExists("mpv");
    bool ytdlp_ok = ymcli::commandExists("yt-dlp");

    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--help" || arg == "-h") {
            std::cout << "\033[1;38;2;217;145;120mymcli\033[0m \033[38;2;96;165;250mv0.1.0\033[0m — terminal client for YouTube Music\n\n";
            std::cout << "\033[1;38;2;57;255;20m$\033[0m \033[1mymcli\033[0m [options]\n\n";
            std::cout << "OPTIONS:\n";
            std::cout << "  \033[38;2;96;165;250m-h, --help\033[0m       Show this help message and exit\n";
            std::cout << "  \033[38;2;96;165;250m-v, --version\033[0m    Show version information\n\n";
            std::cout << "KEYBOARD NAVIGATION:\n";
            std::cout << "  \033[38;2;217;145;120m/\033[0m                Focus search bar\n";
            std::cout << "  \033[38;2;217;145;120mShift+← / Shift+→\033[0m Switch between sidebar and content pane\n";
            std::cout << "  \033[38;2;217;145;120mShift+F / F\033[0m       Toggle fullscreen player with audio visualizer\n";
            std::cout << "  \033[38;2;217;145;120mSpace\033[0m             Play / Pause playback\n";
            std::cout << "  \033[38;2;217;145;120mn / p\033[0m             Next / Previous track\n";
            std::cout << "  \033[38;2;217;145;120m1 - 7\033[0m             Direct jump to view tab\n";
            std::cout << "  \033[38;2;217;145;120m?\033[0m                 Toggle full shortcuts cheatsheet\n";
            std::cout << "  \033[38;2;217;145;120mq\033[0m                 Quit\n" << std::flush;
            return EXIT_SUCCESS;
        }
        if (arg == "--version" || arg == "-v") {
            std::cout << "ymcli 0.1.0\n" << std::flush;
            return EXIT_SUCCESS;
        }
    }

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
