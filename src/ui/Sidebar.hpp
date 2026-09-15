#pragma once

#include <ftxui/component/component.hpp>
#include <vector>
#include <string>
#include <functional>

namespace ymcli::ui {

class Sidebar {
public:
    using OnSelectCallback = std::function<void(int index)>;

    explicit Sidebar(OnSelectCallback on_select = nullptr);
    Sidebar(int* legacy_active_screen);

    ftxui::Component GetComponent();
    int selected() const { return sidebar_index_; }
    void SetSelectedIndex(int index);

private:
    int sidebar_index_ = 0;
    OnSelectCallback on_select_;
    std::vector<std::string> items_ = {
        "1  Home", "2  Search", "3  Library", "4  Queue", "5  History", "6  Favorites", "7  Settings"
    };
    ftxui::Component menu_;
    ftxui::Component component_;
};

} // namespace ymcli::ui
