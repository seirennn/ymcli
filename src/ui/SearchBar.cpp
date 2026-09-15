#include "SearchBar.hpp"
#include "Theme.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component_options.hpp>

namespace ymcli::ui {

SearchBar::SearchBar(std::function<void(std::string)> on_search) {
    ftxui::InputOption opt;
    opt.placeholder = "Search YouTube Music...";
    opt.on_enter = [this, on_search] {
        if (!content_.empty()) {
            on_search(content_);
        }
    };

    input_ = ftxui::Input(&content_, opt);

    component_ = ftxui::Renderer(input_, [this] {
        return ftxui::vbox({
            ftxui::hbox({
                ftxui::text(" 🔍 ") | ftxui::color(Theme::DimAccent),
                input_->Render() | ftxui::flex
            }) | ftxui::bgcolor(Theme::Surface),
            ftxui::separator() | ftxui::color(Theme::Border)
        });
    });

    component_ |= ftxui::CatchEvent([](ftxui::Event event) {
        if (event == ftxui::Event::Escape) {
            return false;
        }
        return false;
    });
}

ftxui::Component SearchBar::GetComponent() { return component_; }

} // namespace ymcli::ui
