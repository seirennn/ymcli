#include "HomeScreen.hpp"
#include "../Theme.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component_options.hpp>

namespace ymcli::ui::screens {

HomeScreen::HomeScreen() {
    menu_ = ftxui::Menu(
        (std::vector<std::string>*)&recent_plays_, // A bit of a hack, better to use custom rendering for menu
        &selected_
    );

    // Let's replace the menu_ with a custom renderer that uses a Vertical container of buttons or just custom event handling
    auto container = ftxui::Container::Vertical({});
    
    component_ = ftxui::Renderer(container, [this] {
        auto header = ftxui::vbox({
            Theme::heading("Welcome to ymcli") | ftxui::center,
            Theme::subtext("Press / to search, or browse with the sidebar") | ftxui::center
        }) | ftxui::borderEmpty | ftxui::yframe;

        ftxui::Elements play_elements;
        if (recent_plays_.empty()) {
            play_elements.push_back(Theme::subtext("No recent plays yet") | ftxui::center);
        } else {
            for (size_t i = 0; i < recent_plays_.size(); ++i) {
                const auto& play = recent_plays_[i];
                auto entry = ftxui::hbox({
                    ftxui::text(play.title) | ftxui::color(Theme::TextPrimary) | ftxui::flex,
                    ftxui::text(play.artist) | ftxui::color(Theme::TextSecondary)
                });
                if (static_cast<int>(i) == selected_) {
                    entry = entry | ftxui::focus | ftxui::color(Theme::PlayingIndicator); // simple focus styling
                }
                play_elements.push_back(entry);
            }
        }

        return ftxui::vbox({
            header,
            ftxui::separatorEmpty(),
            Theme::heading("Recent Plays"),
            ftxui::separator() | ftxui::color(Theme::Border),
            ftxui::vbox(play_elements) | ftxui::yframe | ftxui::flex
        });
    });

    component_ |= ftxui::CatchEvent([this](ftxui::Event event) {
        if (event == ftxui::Event::Character('j') || event == ftxui::Event::ArrowDown) {
            if (!recent_plays_.empty()) {
                selected_ = std::min((int)recent_plays_.size() - 1, selected_ + 1);
                return true;
            }
        }
        if (event == ftxui::Event::Character('k') || event == ftxui::Event::ArrowUp) {
            if (!recent_plays_.empty()) {
                selected_ = std::max(0, selected_ - 1);
                return true;
            }
        }
        if (event == ftxui::Event::Return) {
            // trigger play
            return true;
        }
        return false;
    });
}

ftxui::Component HomeScreen::GetComponent() { return component_; }
void HomeScreen::SetRecentPlays(const std::vector<RecentPlay>& plays) { recent_plays_ = plays; selected_ = 0; }

} // namespace ymcli::ui::screens
