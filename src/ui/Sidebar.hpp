#pragma once

#include <ftxui/component/component.hpp>
#include <vector>
#include <string>

namespace ymcli::ui {

class Sidebar {
public:
    Sidebar(int* active_screen);
    ftxui::Component GetComponent();
    int& selected();

private:
    int dummy_selected_ = 0;
    int* selected_ptr_ = nullptr;
    std::vector<std::string> items_ = {
        "  Home", "  Search", "  Library", "  Queue", "  History", "  Favorites", "  Settings"
    };
    ftxui::Component menu_;
    ftxui::Component component_;
};

} // namespace ymcli::ui
