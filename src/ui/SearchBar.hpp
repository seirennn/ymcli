#pragma once
#include <ftxui/component/component.hpp>
#include <string>
#include <functional>

namespace ymcli::ui {

class SearchBar {
public:
    SearchBar(std::function<void(std::string)> on_search);
    ftxui::Component GetComponent();

private:
    std::string content_;
    ftxui::Component input_;
    ftxui::Component component_;
};

} // namespace ymcli::ui
