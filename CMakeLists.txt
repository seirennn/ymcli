cmake_minimum_required(VERSION 3.16)
project(ymcli
    VERSION 0.1.0
    DESCRIPTION "Terminal YouTube Music Client"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(APPLE)
    execute_process(
        COMMAND brew --prefix
        OUTPUT_VARIABLE HOMEBREW_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(HOMEBREW_PREFIX)
        list(APPEND CMAKE_PREFIX_PATH "${HOMEBREW_PREFIX}")
        set(OPENSSL_ROOT_DIR "${HOMEBREW_PREFIX}/opt/openssl@3")
        set(ENV{PKG_CONFIG_PATH} "${HOMEBREW_PREFIX}/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
    endif()
endif()

include(FetchContent)

FetchContent_Declare(
    ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
    GIT_TAG        v5.0.0
    GIT_SHALLOW    TRUE
)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json
    GIT_TAG        v3.11.3
    GIT_SHALLOW    TRUE
)

FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib
    GIT_TAG        v0.18.1
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(ftxui json httplib)

find_package(PkgConfig REQUIRED)

find_path(MPV_INCLUDE_DIR NAMES mpv/client.h
    PATHS 
        "${HOMEBREW_PREFIX}/include" 
        "${HOMEBREW_PREFIX}/Cellar/mpv/0.41.0_9/include"
        /opt/homebrew/include 
        /usr/local/include 
        /usr/include
)
find_library(MPV_LIBRARY NAMES mpv
    PATHS 
        "${HOMEBREW_PREFIX}/lib" 
        "${HOMEBREW_PREFIX}/Cellar/mpv/0.41.0_9/lib"
        /opt/homebrew/lib 
        /usr/local/lib 
        /usr/lib
)

if(MPV_INCLUDE_DIR AND MPV_LIBRARY)
    add_library(mpv_lib INTERFACE)
    target_include_directories(mpv_lib INTERFACE "${MPV_INCLUDE_DIR}")
    target_link_libraries(mpv_lib INTERFACE "${MPV_LIBRARY}")
else()
    pkg_check_modules(MPV REQUIRED IMPORTED_TARGET mpv)
    add_library(mpv_lib ALIAS PkgConfig::MPV)
endif()

find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(ymcli
    src/main.cpp
    src/app/App.cpp
    src/audio/AudioEngine.cpp
    src/audio/Queue.cpp
    src/api/InnerTube.cpp
    src/api/CookieExtractor.cpp
    src/api/ImageCache.cpp
    src/db/Database.cpp
    src/util/Format.cpp
    src/util/Platform.cpp
    src/ui/Layout.cpp
    src/ui/Sidebar.cpp
    src/ui/NowPlaying.cpp
    src/ui/SearchBar.cpp
    src/ui/screens/HomeScreen.cpp
    src/ui/screens/SearchScreen.cpp
    src/ui/screens/AlbumScreen.cpp
    src/ui/screens/ArtistScreen.cpp
    src/ui/screens/PlaylistScreen.cpp
    src/ui/screens/QueueScreen.cpp
    src/ui/screens/HistoryScreen.cpp
    src/ui/screens/FavoritesScreen.cpp
    src/ui/screens/SettingsScreen.cpp
)

target_include_directories(ymcli PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_compile_definitions(ymcli PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)

target_link_libraries(ymcli PRIVATE
    ftxui::screen
    ftxui::dom
    ftxui::component
    nlohmann_json::nlohmann_json
    httplib::httplib
    mpv_lib
    SQLite::SQLite3
    OpenSSL::SSL
    OpenSSL::Crypto
)

install(TARGETS ymcli DESTINATION bin)
