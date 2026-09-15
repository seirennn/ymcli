# ymcli

I built ymcli because I work in the terminal all day, and switching over to a browser just to search for a track or pause a song felt clunky and slow. Navigating and searching with vim keys in the terminal is just so much faster. It was also a fun project to get better at writing modern C++.

It streams audio directly through libmpv, talks to YouTube Music's InnerTube API, and renders a clean, quiet interface with FTXUI.


## Prerequisites

You'll need mpv, yt-dlp, and cmake:

  brew install mpv yt-dlp cmake openssl@3


## Building & Installing

  git clone https://github.com/seirennn/ymcli.git
  cd ymcli
  mkdir build && cd build
  cmake ..
  make -j$(sysctl -n hw.ncpu)
  sudo make install


## Controls

Press ? inside ymcli anytime to open the shortcuts cheatsheet.

Playback:
  space       Play or pause
  n           Next track
  p           Previous track (or restarts track if > 3s)
  < / >       Seek backward / forward 10s
  + / -       Volume up / down
  m           Toggle mute
  s           Toggle shuffle
  r           Cycle repeat mode

Navigation:
  /               Focus search bar
  j / k           Move up / down
  Enter           Play song & queue upcoming tracks
  Shift + ← / →   Switch pane (sidebar ↔ main content)
  Tab             Toggle pane focus or section
  Shift + F / F   Fullscreen player & atmospheric audio visualizer
  1 to 7          Switch tabs (Home, Search, Library, Queue, History, Favorites, Settings)
  ?               Shortcuts cheatsheet
  Esc             Unfocus / Go back
  q               Quit

Track Actions:
  a           Add highlighted track to current queue
  l or +      Add track to a playlist
  f           Save / Favorite track
  P           Play all tracks (in album / playlist)
  A           Add all tracks to play queue
  d           Delete track or local playlist


## Library & Playlists

Tab 3 (Library) loads your YouTube Music account playlists (like Liked Music, custom playlists, etc.) as well as your local playlists saved in ymcli.

You can press n in the Library tab to create a new local playlist, or press l on any song anywhere in the app to save it into a playlist.


## Authentication

You can use ymcli completely unauthenticated for public searches, but logging in lets you load your saved playlists, liked music, and history.

The easiest way is 1-click auto-login:
  1. Go to Settings (press 7).
  2. Hit the Auto-Detect Browser button. It reads your local browser session (Arc, Chrome, Brave, Edge, Firefox, etc.) and signs you in immediately.

If you prefer manual cookies:
  1. Open music.youtube.com in your browser.
  2. Open DevTools (F12) -> Network tab, click any request to youtubei.
  3. Copy your Cookie string, paste it in Settings, and hit Authenticate.

Sessions and cookies are saved in ~/.config/ymcli/.


## Tech Stack

  C++20
  FTXUI for terminal UI
  libmpv & yt-dlp for audio streaming
  SQLite3 for local history, favorites, and playlists
  cpp-httplib & nlohmann/json for API requests
