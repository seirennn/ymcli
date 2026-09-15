#include "SettingsScreen.hpp"
#include "../Theme.hpp"
#include <ftxui/dom/elements.hpp>

namespace ymcli::ui::screens {

SettingsScreen::SettingsScreen(AuthCallback auth_cb, AutoDetectCallback auto_cb) {
    volume_slider_ = ftxui::Slider("Volume: ", &volume_, 0, 100, 5);
    quality_toggle_ = ftxui::Toggle(&quality_options_, &quality_index_);

    cookie_field_ = ftxui::Input(&cookie_input_, "paste raw Cookie header string (SAPISID=...)");

    auth_button_ = ftxui::Button(" [Authenticate Cookie] ", [this, auth_cb] {
        if (auth_cb && !cookie_input_.empty()) {
            is_authenticated_ = auth_cb(cookie_input_);
            if (is_authenticated_) {
                cookie_input_.clear();
                status_message_ = "[ok] Authenticated successfully with cookie.";
                status_is_success_ = true;
            } else {
                status_message_ = "[error] Failed. Cookie must contain SAPISID or __Secure-3PAPISID.";
                status_is_success_ = false;
            }
        }
    });

    auto_detect_button_ = ftxui::Button(" [Auto-Detect from Browser Session] ", [this, auto_cb] {
        if (auto_cb) {
            std::string browser;
            is_authenticated_ = auto_cb(browser);
            if (is_authenticated_) {
                status_message_ = "[ok] Session imported from " + (browser.empty() ? "browser" : browser) + ".";
                status_is_success_ = true;
            } else {
                status_message_ = "[error] No active YouTube Music session found in browser profiles.";
                status_is_success_ = false;
            }
        }
    });

    btn_clear_history_ = ftxui::Button(" [Clear History] ", [] {
    });

    auto container = ftxui::Container::Vertical({
        auto_detect_button_,
        cookie_field_,
        auth_button_,
        volume_slider_,
        quality_toggle_,
        btn_clear_history_
    });

    component_ = ftxui::Renderer(container, [this, container] {
        std::string auth_badge = is_authenticated_ ? "[connected]" : "[not connected]";
        auto auth_color = is_authenticated_ ? ftxui::color(Theme::PlayingIndicator) : ftxui::color(Theme::TextTertiary);

        ftxui::Elements auth_section_elements = {
            ftxui::hbox({
                ftxui::text("ACCOUNT AUTHENTICATION") | ftxui::bold | ftxui::color(Theme::Accent),
                ftxui::filler(),
                ftxui::text(auth_badge) | auth_color | ftxui::bold
            }),
            ftxui::separator() | ftxui::color(Theme::Border),
            ftxui::text("1-click session import (Firefox, Arc, Chrome, Brave, Edge, Zen):") | ftxui::color(Theme::TextSecondary),
            auto_detect_button_->Render() | ftxui::color(Theme::Accent)
        };

        if (!status_message_.empty()) {
            auto msg_color = status_is_success_ ? ftxui::color(Theme::PlayingIndicator) : ftxui::color(Theme::Accent);
            auth_section_elements.push_back(
                ftxui::text("  " + status_message_) | msg_color
            );
        }

        auth_section_elements.push_back(ftxui::text(""));
        auth_section_elements.push_back(ftxui::text("Manual cookie fallback:") | ftxui::color(Theme::TextSecondary));
        auth_section_elements.push_back(cookie_field_->Render() | ftxui::bgcolor(Theme::Surface));
        auth_section_elements.push_back(auth_button_->Render() | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 28));

        return ftxui::vbox({
            ftxui::text("SETTINGS") | ftxui::bold | ftxui::color(Theme::Accent),
            ftxui::separator() | ftxui::color(Theme::Border),
            
            ftxui::vbox(std::move(auth_section_elements)) | ftxui::bgcolor(Theme::SecondaryBg) | ftxui::borderRounded | ftxui::color(Theme::Border),

            ftxui::text(""),

            ftxui::vbox({
                ftxui::text("AUDIO OPTIONS") | ftxui::bold | ftxui::color(Theme::Accent),
                ftxui::separator() | ftxui::color(Theme::Border),
                volume_slider_->Render(),
                ftxui::hbox({
                    ftxui::text("Stream Quality: ") | ftxui::color(Theme::TextSecondary),
                    quality_toggle_->Render()
                })
            }) | ftxui::bgcolor(Theme::SecondaryBg) | ftxui::borderRounded | ftxui::color(Theme::Border),

            ftxui::text(""),

            ftxui::vbox({
                ftxui::text("ABOUT") | ftxui::bold | ftxui::color(Theme::Accent),
                ftxui::separator() | ftxui::color(Theme::Border),
                ftxui::text("ymcli — minimal C++20 terminal audio client for YouTube Music") | ftxui::color(Theme::TextSecondary),
                ftxui::text("Data: ~/.local/share/ymcli   Config: ~/.config/ymcli") | ftxui::color(Theme::TextTertiary)
            }) | ftxui::bgcolor(Theme::SecondaryBg) | ftxui::borderRounded | ftxui::color(Theme::Border)
        }) | ftxui::bgcolor(Theme::Background);
    });
}

ftxui::Component SettingsScreen::GetComponent() { return component_; }
void SettingsScreen::setAuthStatus(bool authenticated) { is_authenticated_ = authenticated; }

} // namespace ymcli::ui::screens
