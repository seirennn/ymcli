#include "Layout.hpp"
#include "Theme.hpp"
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/component_options.hpp>

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

    ftxui::ResizableSplitOption split_opt;
    split_opt.main = sidebar.GetComponent();
    split_opt.back = right_renderer;
    split_opt.direction = ftxui::Direction::Left;
    split_opt.main_size = &sidebar_width_;
    split_opt.separator_func = [] {
        return ftxui::separator() | ftxui::color(Theme::Border);
    };
    auto split = ftxui::ResizableSplit(std::move(split_opt));
    
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
