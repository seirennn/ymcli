# ymcli

A lightweight, high-performance terminal client for YouTube Music written in modern C++20.

Designed for fast keyboard-driven browsing, instant YouTube Music search, and continuous audio streaming via `libmpv` without opening a browser or electron window.

```
┌─────────────┬───────────────────────────────────────────────────────────┐
│             │  🔍 Search YouTube Music...                               │
│  ▸ Home     ├───────────────────────────────────────────────────────────┤
│    Search   │                                                           │
│    Library  │  Daft Punk — One More Time                       3:55     │
│    Queue    │  Daft Punk — Harder, Better, Faster, Stronger    3:44     │
│    History  │  Daft Punk — Around the World                    7:09     │
│    Favorites│                                                           │
│    Settings │                                                           │
├─────────────┴───────────────────────────────────────────────────────────┤
│  ▶  One More Time — Daft Punk           ━━━━━━━━━━━━━━━░░  1:42 / 3:55   │
│     Discovery                           ◁   ▶   ▷    🔀   🔁   Vol 80%  │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## Features

- **Native YouTube Music InnerTube Integration**: Directly interfaces with YouTube Music's `/youtubei/v1` API endpoints for fast, lightweight search and discovery.
- **Headless Audio Engine**: Powered by `libmpv` and `yt-dlp`. Plays audio-only DASH streams (Opus/AAC) directly from YouTube CDNs with forward buffering.
- **Browser Authentication**: Supports browser cookie authentication (`SAPISIDHASH` SHA-1 challenge) to access your liked songs, private playlists, and personal play history.
- **Local SQLite Persistence**: Tracks search history, play history, and favorited songs locally (`~/.local/share/ymcli/ymcli.db`).
- **Dark Atmospheric UI**: Built with FTXUI. Solid `#121212` background, restrained steel-blue accent colors, zero ornamental bloat.
- **Full Keyboard Controls**: Vim navigation (`j`/`k`), global hotkeys (`space`, `n`, `p`, `+`, `-`, `m`, `s`, `r`), search trigger (`/`).

---

## Prerequisites

- macOS or Linux
- **CMake** 3.16+
- **Apple Clang** or **GCC** with C++20 support
- **mpv** (`brew install mpv` / `apt install libmpv-dev mpv`)
- **yt-dlp** (`brew install yt-dlp`)
- **OpenSSL** (`brew install openssl@3`)
- **SQLite3**

---

## Build & Install

```bash
git clone https://github.com/Seirennn/ymcli.git
cd ymcli
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu)
sudo make install
```

---

## Keybindings

| Key | Action |
|:---|:---|
| `/` | Focus search bar |
| `space` | Toggle play / pause |
| `n` | Next track |
| `p` | Previous track |
| `+` / `=` | Increase volume |
| `-` | Decrease volume |
| `m` | Toggle mute |
| `s` | Toggle shuffle |
| `r` | Cycle repeat mode (Off → All → One) |
| `>` / `<` | Seek forward / backward 10s |
| `1`–`7` | Sidebar shortcuts (Home, Search, Library, Queue, History, Favorites, Settings) |
| `j` / `k` | Navigate lists |
| `q` | Quit |

---

## Authentication Setup

1. Open `music.youtube.com` in your web browser while logged into your Google account.
2. Open Developer Tools (F12) → Network tab.
3. Filter by `youtubei` and click any request.
4. Copy the raw `Cookie` request header string.
5. In `ymcli`, navigate to **Settings** (`7`), paste the cookie string into the input box, and press **Authenticate**.
6. Credentials are saved locally to `~/.config/ymcli/auth.json`.

---

## License

MIT License. See [LICENSE](LICENSE) for details.
