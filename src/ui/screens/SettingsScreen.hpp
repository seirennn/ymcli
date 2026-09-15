#pragma once

#include <ftxui/component/component.hpp>
#include <vector>
#include <string>
#include <functional>

namespace ymcli::ui::screens {

class SettingsScreen {
public:
    using AuthCallback = std::function<bool(const std::string& cookie)>;
    using AutoDetectCallback = std::function<bool(std::string& out_browser)>;

    SettingsScreen(AuthCallback auth_cb = nullptr, AutoDetectCallback auto_cb = nullptr);
    ftxui::Component GetComponent();

    void setAuthStatus(bool is_authenticated);

private:
    int volume_ = 80;
    int quality_index_ = 0;
    std::vector<std::string> quality_options_ = {"Best (Opus 160k)", "Medium (AAC 128k)", "Low (Opus 50k)"};
    
    std::string cookie_input_;
    bool is_authenticated_ = false;
    std::string status_message_;
    bool status_is_success_ = false;

    ftxui::Component volume_slider_;
    ftxui::Component quality_toggle_;
    ftxui::Component cookie_field_;
    ftxui::Component auth_button_;
    ftxui::Component auto_detect_button_;
    ftxui::Component btn_clear_history_;
    ftxui::Component component_;
};

} // namespace ymcli::ui::screens
