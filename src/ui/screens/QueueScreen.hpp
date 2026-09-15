#pragma once
#include <ftxui/component/component.hpp>
#include <vector>
#include <string>

namespace ymcli::ui::screens {

struct QueueItem { std::string title; std::string artist; bool is_playing = false; };

class QueueScreen {
public:
    QueueScreen();
    ftxui::Component GetComponent();
    
    void SetQueue(const std::vector<QueueItem>& items);

private:
    std::vector<QueueItem> items_;
    int selected_ = 0;
    ftxui::Component component_;
};

} // namespace ymcli::ui::screens
