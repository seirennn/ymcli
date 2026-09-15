#include "AlbumScreen.hpp"
#include "../Theme.hpp"
#include <ftxui/dom/elements.hpp>

namespace ymcli::ui::screens {

AlbumScreen::AlbumScreen() {
    component_ = ftxui::Renderer([this] {
        auto header = ftxui::vbox({
            Theme::heading(title_),
            Theme::subtext(artist_),
            ftxui::text(year_) | ftxui::color(Theme::TextTertiary),
            ftxui::text("[Enter] Play Track  [a] Add to Queue  [P] Play All") | ftxui::color(Theme::DimAccent)
        });

        ftxui::Elements track_elements;
        for (size_t i = 0; i < tracks_.size(); ++i) {
            auto& t = tracks_[i];
            auto el = ftxui::hbox({
                ftxui::text(std::to_string(i + 1) + ". ") | ftxui::color(Theme::TextTertiary),
                ftxui::text(t.title) | ftxui::color(Theme::TextPrimary) | ftxui::flex,
                ftxui::text(t.duration) | ftxui::color(Theme::TextSecondary)
            });
            if (static_cast<int>(i) == selected_) el = el | Theme::focused_style();
            track_elements.push_back(el);
        }

        return ftxui::vbox({
            header,
            ftxui::separator() | ftxui::color(Theme::Border),
            ftxui::vbox(track_elements) | ftxui::yframe | ftxui::flex
        });
    });

    component_ |= ftxui::CatchEvent([this](ftxui::Event event) {
        if (event == ftxui::Event::Character('j')) { selected_ = std::min((int)tracks_.size() - 1, selected_ + 1); return true; }
        if (event == ftxui::Event::Character('k')) { selected_ = std::max(0, selected_ - 1); return true; }
        return false;
    });
}

ftxui::Component AlbumScreen::GetComponent() { return component_; }
void AlbumScreen::SetData(const std::string& title, const std::string& artist, const std::string& year, const std::vector<AlbumTrack>& tracks) {
    title_ = title; artist_ = artist; year_ = year; tracks_ = tracks; selected_ = 0;
}

} // namespace ymcli::ui::screens
