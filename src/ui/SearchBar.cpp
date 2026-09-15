#include "SearchBar.hpp"
#include "Theme.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component_options.hpp>

namespace ymcli::ui {

SearchBar::SearchBar(std::function<void(std::string)> on_search) {
    ftxui::InputOption opt;
    opt.placeholder = "type song, album or artist...";
    opt.on_enter = [this, on_search] {
        if (!content_.empty()) {
            on_search(content_);
        }
    };

    input_ = ftxui::Input(&content_, opt);

    component_ = ftxui::Renderer(input_, [this] {
        bool is_focused = input_ && input_->Focused();
        return ftxui::vbox({
            ftxui::hbox({
                ftxui::text(" $ ") | ftxui::bold | ftxui::color(Theme::CmdPrefix),
                ftxui::text("search") | ftxui::bold | ftxui::color(Theme::Accent),
                ftxui::text(" --query ") | ftxui::color(Theme::KeywordBlue),
                input_->Render() | ftxui::flex,
                ftxui::text(" [Enter: Run  Esc: Cancel] ") | ftxui::color(Theme::TextTertiary)
            }) | ftxui::bgcolor(is_focused ? Theme::Elevated : Theme::Surface),
            ftxui::separator() | ftxui::color(is_focused ? Theme::FocusBorder : Theme::BorderLight)
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
