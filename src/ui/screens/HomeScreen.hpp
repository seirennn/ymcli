#pragma once
#include <ftxui/component/component.hpp>
#include <vector>
#include <string>

namespace ymcli::ui::screens {

struct RecentPlay {
    std::string title;
    std::string artist;
};

class HomeScreen {
public:
    HomeScreen();
    ftxui::Component GetComponent();
    void SetRecentPlays(const std::vector<RecentPlay>& plays);

private:
    std::vector<RecentPlay> recent_plays_;
    int selected_ = 0;
    ftxui::Component menu_;
    ftxui::Component component_;
};

} // namespace ymcli::ui::screens
