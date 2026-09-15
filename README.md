# ymcli

I built ymcli because I find myself working in the terminal all day, and switching back and forth to a web browser just to search for a song or control playback felt slow and unnecessary. Searching, queuing, and playing music directly from my shell with keybindings is so much faster, and building this was a great way for me to get better at modern C++.

It streams YouTube Music audio directly through libmpv and yt-dlp, rendered with a clean FTXUI terminal interface.


## Quick Start

Make sure you have mpv, yt-dlp, and cmake installed:

  brew install mpv yt-dlp cmake openssl@3

Build and install:

  mkdir build && cd build
  cmake ..
  make
  sudo make install


## Keybindings

  /           Focus search bar
  space       Play or pause
  n / p       Next or previous track
  + / -       Volume control
  m           Toggle mute
  s / r       Toggle shuffle / repeat
  1 to 7      Switch tabs (Home, Search, Library, Queue, History, Favorites, Settings)
  j / k       Navigate lists
  q           Quit


## Authenticating

To access your liked songs and playlists:

  1. Open music.youtube.com in your browser.
  2. Open DevTools (F12) -> Network tab, click any request to youtubei.
  3. Copy your raw Cookie header string.
  4. In ymcli, go to Settings (7), paste the cookie, and hit Authenticate.

Cookies are stored locally in ~/.config/ymcli/auth.json.


## Tech Stack

  C++20
  FTXUI for the TUI
  libmpv & yt-dlp for audio playback
  SQLite3 for local history and favorites
  cpp-httplib & nlohmann/json for API requests
