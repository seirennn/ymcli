#pragma once
#include <ftxui/component/component.hpp>
#include <vector>
#include <string>

namespace ymcli::ui::screens {

struct HistoryItem { std::string title; std::string artist; std::string time; };

class HistoryScreen {
public:
    HistoryScreen();
    ftxui::Component GetComponent();
    void SetHistory(const std::vector<HistoryItem>& items);

private:
    std::vector<HistoryItem> items_;
    int selected_ = 0;
    ftxui::Component component_;
};

} // namespace ymcli::ui::screens
