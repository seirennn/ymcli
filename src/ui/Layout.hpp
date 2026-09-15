#pragma once
#include <ftxui/component/component.hpp>
#include "Sidebar.hpp"
#include "SearchBar.hpp"
#include "NowPlaying.hpp"

namespace ymcli::ui {

class Layout {
public:
    Layout(Sidebar& sidebar, SearchBar& search_bar, ftxui::Component content_area, NowPlaying& now_playing);
    ftxui::Component GetComponent();

private:
    ftxui::Component root_;
    int sidebar_width_ = 20;
};

} // namespace ymcli::ui
