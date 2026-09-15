#include "PlaylistScreen.hpp"
#include "../Theme.hpp"
#include <ftxui/dom/elements.hpp>

namespace ymcli::ui::screens {

PlaylistScreen::PlaylistScreen() {
    component_ = ftxui::Renderer([this] {
        auto header = ftxui::vbox({
            Theme::heading(title_),
            Theme::subtext(author_),
            ftxui::text(count_ + " tracks") | ftxui::color(Theme::TextTertiary),
            ftxui::text("[P] Play All  [A] Add All to Queue") | ftxui::color(Theme::DimAccent)
        });

        ftxui::Elements track_elements;
        for (size_t i = 0; i < tracks_.size(); ++i) {
            auto el = ftxui::hbox({
                ftxui::text(tracks_[i].title) | ftxui::color(Theme::TextPrimary) | ftxui::flex,
                ftxui::text(tracks_[i].artist) | ftxui::color(Theme::TextSecondary)
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

ftxui::Component PlaylistScreen::GetComponent() { return component_; }
void PlaylistScreen::SetData(const std::string& title, const std::string& author, const std::string& count, const std::vector<PlaylistTrack>& tracks) {
    title_ = title; author_ = author; count_ = count; tracks_ = tracks; selected_ = 0;
}

} // namespace ymcli::ui::screens
