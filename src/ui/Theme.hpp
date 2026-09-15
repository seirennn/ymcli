#pragma once
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <string>

namespace ymcli::ui::Theme {

inline const ftxui::Color Background = ftxui::Color::RGB(18, 18, 18);
inline const ftxui::Color SecondaryBg = ftxui::Color::RGB(24, 24, 24);
inline const ftxui::Color Surface = ftxui::Color::RGB(30, 30, 30);
inline const ftxui::Color Elevated = ftxui::Color::RGB(40, 40, 40);

inline const ftxui::Color TextPrimary = ftxui::Color::RGB(210, 210, 210);
inline const ftxui::Color TextSecondary = ftxui::Color::RGB(140, 140, 140);
inline const ftxui::Color TextTertiary = ftxui::Color::RGB(90, 90, 90);

inline const ftxui::Color Accent = ftxui::Color::RGB(120, 160, 200);
inline const ftxui::Color DimAccent = ftxui::Color::RGB(80, 110, 140);
inline const ftxui::Color PlayingIndicator = ftxui::Color::RGB(100, 180, 120);

inline const ftxui::Color Border = ftxui::Color::RGB(45, 45, 45);
inline const ftxui::Color FocusBorder = ftxui::Color::RGB(70, 70, 70);

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

} // namespace ymcli::ui::Theme
