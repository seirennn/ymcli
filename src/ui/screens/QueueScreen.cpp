#include "QueueScreen.hpp"
#include "../Theme.hpp"
#include <ftxui/dom/elements.hpp>

namespace ymcli::ui::screens {

QueueScreen::QueueScreen() {
    component_ = ftxui::Renderer([this] {
        ftxui::Elements elements;
        for (size_t i = 0; i < items_.size(); ++i) {
            auto& item = items_[i];
            auto el = ftxui::hbox({
                ftxui::text(item.title) | ftxui::color(item.is_playing ? Theme::PlayingIndicator : Theme::TextPrimary) | ftxui::flex,
                ftxui::text(item.artist) | ftxui::color(Theme::TextSecondary)
            });
            if (static_cast<int>(i) == selected_) el = el | Theme::focused_style();
            elements.push_back(el);
        }

        return ftxui::vbox({
            Theme::heading("Current Queue"),
            ftxui::text("[Shift+j/k] Move  [d] Delete  [C] Clear  [Enter] Jump") | ftxui::color(Theme::DimAccent),
            ftxui::separator() | ftxui::color(Theme::Border),
            ftxui::vbox(elements) | ftxui::yframe | ftxui::flex
        });
    });

    component_ |= ftxui::CatchEvent([this](ftxui::Event event) {
        if (event == ftxui::Event::Character('j')) { selected_ = std::min((int)items_.size() - 1, selected_ + 1); return true; }
        if (event == ftxui::Event::Character('k')) { selected_ = std::max(0, selected_ - 1); return true; }
        if (event == ftxui::Event::Character('J')) { /* move down logic */ return true; }
        if (event == ftxui::Event::Character('K')) { /* move up logic */ return true; }
        if (event == ftxui::Event::Character('d')) { /* delete */ return true; }
        if (event == ftxui::Event::Character('C')) { /* clear */ return true; }
        return false;
    });
}

ftxui::Component QueueScreen::GetComponent() { return component_; }
void QueueScreen::SetQueue(const std::vector<QueueItem>& items) { items_ = items; selected_ = 0; }

} // namespace ymcli::ui::screens
