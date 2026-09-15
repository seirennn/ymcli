#include "Layout.hpp"
#include "Theme.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/event.hpp>

namespace ymcli::ui {

Layout::Layout(Sidebar& sidebar, SearchBar& search_bar, ftxui::Component content_area, NowPlaying& now_playing) {
    auto right_panel = ftxui::Container::Vertical({
        search_bar.GetComponent(),
        content_area,
        now_playing.GetComponent()
    });

    auto right_renderer = ftxui::Renderer(right_panel, [search_bar, content_area, now_playing]() mutable {
        return ftxui::vbox({
            search_bar.GetComponent()->Render(),
            content_area->Render() | ftxui::flex,
            ftxui::separator() | ftxui::color(Theme::Border),
            now_playing.GetComponent()->Render()
        }) | ftxui::bgcolor(Theme::Background);
    });

    auto split = ftxui::ResizableSplitLeft(sidebar.GetComponent(), right_renderer, &sidebar_width_);
    
    root_ = ftxui::Renderer(split, [split] {
        return split->Render() | ftxui::bgcolor(Theme::Background);
    });

    root_ |= ftxui::CatchEvent([search_bar_comp = search_bar.GetComponent()](ftxui::Event event) {
        if (event == ftxui::Event::Character('/')) {
            search_bar_comp->TakeFocus();
            return true;
        }
        return false;
    });
}

ftxui::Component Layout::GetComponent() { return root_; }

} // namespace ymcli::ui
