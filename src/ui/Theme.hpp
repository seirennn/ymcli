#pragma once
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <string>

namespace ymcli::ui::Theme {

// Terminal UI Design System - Dark Mode Palette
// Backgrounds
inline const ftxui::Color Background   = ftxui::Color::RGB(10, 10, 10);    // --bg-main: #0a0a0a
inline const ftxui::Color SecondaryBg  = ftxui::Color::RGB(26, 26, 26);    // --secondary: #1a1a1a
inline const ftxui::Color Surface      = ftxui::Color::RGB(17, 17, 17);    // --bg-card: #111111
inline const ftxui::Color Elevated     = ftxui::Color::RGB(38, 38, 38);    // --muted: #262626
inline const ftxui::Color CodeBg       = ftxui::Color::RGB(24, 24, 27);    // --bg-code: #18181b

// Primary & Accents (Warm Terracotta & Terminal Highlights)
inline const ftxui::Color Primary          = ftxui::Color::RGB(217, 145, 120); // --primary: #d99178 (warm terracotta)
inline const ftxui::Color Accent           = ftxui::Color::RGB(217, 145, 120); // main accent
inline const ftxui::Color PrimaryDark      = ftxui::Color::RGB(197, 127, 102); // --primary-dark: #c57f66
inline const ftxui::Color DimAccent        = ftxui::Color::RGB(181, 104, 80);  // --primary-dark: #b56850
inline const ftxui::Color WarmRed          = ftxui::Color::RGB(200, 90, 63);   // --warm-red: #c85a3f

// Terminal Syntax & Semantic Colors
inline const ftxui::Color CmdPrefix        = ftxui::Color::RGB(57, 255, 20);   // --cmd-prefix: #39ff14 (fluorescent green $)
inline const ftxui::Color Success          = ftxui::Color::RGB(34, 197, 94);   // --success: #22c55e (green)
inline const ftxui::Color PlayingIndicator = ftxui::Color::RGB(34, 197, 94);   // terminal green
inline const ftxui::Color KeywordBlue      = ftxui::Color::RGB(96, 165, 250);  // --blue: #60a5fa
inline const ftxui::Color Warning          = ftxui::Color::RGB(245, 158, 11);  // --warning: #f59e0b
inline const ftxui::Color Danger           = ftxui::Color::RGB(239, 68, 68);   // --danger: #ef4444

// macOS Window Control Dots
inline const ftxui::Color DotRed           = ftxui::Color::RGB(255, 95, 87);   // --dot-red: #ff5f57
inline const ftxui::Color DotYellow        = ftxui::Color::RGB(254, 188, 46);  // --dot-yellow: #febc2e
inline const ftxui::Color DotGreen         = ftxui::Color::RGB(40, 200, 64);   // --dot-green: #28c840

// Text Colors
inline const ftxui::Color TextPrimary      = ftxui::Color::RGB(237, 237, 237); // --text-primary: #ededed
inline const ftxui::Color TextSecondary    = ftxui::Color::RGB(163, 163, 163); // --text-secondary: #a3a3a3
inline const ftxui::Color TextTertiary     = ftxui::Color::RGB(96, 96, 104);   // --text-muted: #606068

// Borders
inline const ftxui::Color Border           = ftxui::Color::RGB(96, 96, 104);   // --border: #606068
inline const ftxui::Color BorderLight      = ftxui::Color::RGB(39, 39, 42);    // --border-light: #27272a
inline const ftxui::Color FocusBorder      = ftxui::Color::RGB(217, 145, 120); // --ring: #d99178

// UI Helpers
inline ftxui::Element window_dots() {
    return ftxui::hbox({
        ftxui::text("● ") | ftxui::color(DotRed),
        ftxui::text("● ") | ftxui::color(DotYellow),
        ftxui::text("●")  | ftxui::color(DotGreen)
    });
}

inline ftxui::Element themed_border(ftxui::Element inner) {
    return ftxui::borderRounded(std::move(inner)) | ftxui::color(Border);
}

inline ftxui::Element heading(const std::string& text) {
    return ftxui::text(text) | ftxui::bold | ftxui::color(TextPrimary);
}

inline ftxui::Element subtext(const std::string& text) {
    return ftxui::text(text) | ftxui::color(TextSecondary);
}

inline ftxui::Element accent_text(const std::string& text) {
    return ftxui::text(text) | ftxui::color(Accent);
}

inline ftxui::Decorator focused_style() {
    return ftxui::focus | ftxui::color(FocusBorder);
}

inline ftxui::Element cmd_header(const std::string& cmd, const std::string& flag = "", const std::string& comment = "") {
    ftxui::Elements line;
    line.push_back(ftxui::text("$ ") | ftxui::bold | ftxui::color(CmdPrefix));
    line.push_back(ftxui::text(cmd) | ftxui::bold | ftxui::color(Accent));
    if (!flag.empty()) {
        line.push_back(ftxui::text(" " + flag) | ftxui::color(KeywordBlue));
    }
    if (!comment.empty()) {
        line.push_back(ftxui::text("  // " + comment) | ftxui::color(TextTertiary));
    }
    return ftxui::hbox(std::move(line));
}

} // namespace ymcli::ui::Theme
