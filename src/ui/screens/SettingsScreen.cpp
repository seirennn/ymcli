#include "SettingsScreen.hpp"
#include "../Theme.hpp"
#include <ftxui/dom/elements.hpp>

namespace ymcli::ui::screens {

SettingsScreen::SettingsScreen(AuthCallback auth_cb, AutoDetectCallback auto_cb) {
    volume_slider_ = ftxui::Slider("Volume: ", &volume_, 0, 100, 5);
    quality_toggle_ = ftxui::Toggle(&quality_options_, &quality_index_);

    cookie_field_ = ftxui::Input(&cookie_input_, "Or paste raw Cookie string here (SAPISID=...)");

    auth_button_ = ftxui::Button(" Manual Login ", [this, auth_cb] {
        if (auth_cb && !cookie_input_.empty()) {
            is_authenticated_ = auth_cb(cookie_input_);
            if (is_authenticated_) {
                cookie_input_.clear();
                status_message_ = "✓ Successfully authenticated with provided cookie!";
                status_is_success_ = true;
            } else {
                status_message_ = "✕ Authentication failed. Please ensure your cookie contains SAPISID or __Secure-3PAPISID.";
                status_is_success_ = false;
            }
        }
    });

    auto_detect_button_ = ftxui::Button(" ⚡ Auto-Detect & Login from Browser ", [this, auto_cb] {
        if (auto_cb) {
            std::string browser;
            is_authenticated_ = auto_cb(browser);
            if (is_authenticated_) {
                status_message_ = "✓ Successfully detected and imported session" + 
                                  (browser.empty() ? "" : " from " + browser) + "!";
                status_is_success_ = true;
            } else {
                status_message_ = "✕ No active YouTube Music session found in browser profiles.";
                status_is_success_ = false;
            }
        }
    });

    btn_clear_history_ = ftxui::Button(" Clear History ", [] {
        // Clear history action
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
        std::string auth_badge = is_authenticated_ ? " [Authenticated] " : " [Unauthenticated] ";
        auto auth_color = is_authenticated_ ? ftxui::color(Theme::PlayingIndicator) : ftxui::color(Theme::TextTertiary);

        ftxui::Elements auth_section_elements = {
            ftxui::text("Account Authentication") | ftxui::bold | ftxui::color(Theme::TextPrimary),
            ftxui::hbox({
                ftxui::text("Status: ") | ftxui::color(Theme::TextSecondary),
                ftxui::text(auth_badge) | auth_color | ftxui::bold
            }),
            ftxui::text("Auto-Detect extracts active YouTube Music cookies directly from your Firefox, Arc, Chrome, Brave, Edge, or Zen profile.") | ftxui::color(Theme::TextTertiary),
            auto_detect_button_->Render() | ftxui::color(Theme::Accent)
        };

        if (!status_message_.empty()) {
            auto msg_color = status_is_success_ ? ftxui::color(Theme::PlayingIndicator) : ftxui::color(Theme::Accent);
            auth_section_elements.push_back(
                ftxui::text(status_message_) | msg_color | ftxui::bold
            );
        }

        auth_section_elements.push_back(ftxui::separator() | ftxui::color(Theme::Border));
        auth_section_elements.push_back(cookie_field_->Render() | ftxui::borderRounded | ftxui::color(Theme::Border));
        auth_section_elements.push_back(auth_button_->Render() | ftxui::size(ftxui::WIDTH, ftxui::EQUAL, 20));

        return ftxui::vbox({
            ftxui::text("SETTINGS") | ftxui::bold | ftxui::color(Theme::Accent),
            ftxui::separator() | ftxui::color(Theme::Border),
            
            ftxui::vbox(std::move(auth_section_elements)) | ftxui::borderRounded | ftxui::color(Theme::Border),

            ftxui::separator() | ftxui::color(Theme::Border),

            ftxui::vbox({
                ftxui::text("Audio Options") | ftxui::bold | ftxui::color(Theme::TextPrimary),
                volume_slider_->Render(),
                ftxui::hbox({
                    ftxui::text("Stream Quality: ") | ftxui::color(Theme::TextSecondary),
                    quality_toggle_->Render()
                })
            }) | ftxui::borderRounded | ftxui::color(Theme::Border),

            ftxui::separator() | ftxui::color(Theme::Border),

            ftxui::vbox({
                ftxui::text("About ymcli") | ftxui::bold | ftxui::color(Theme::TextPrimary),
                ftxui::text("Version: 0.1.0") | ftxui::color(Theme::TextSecondary),
                ftxui::text("Built with FTXUI, libmpv, SQLite3, cpp-httplib") | ftxui::color(Theme::TextTertiary)
            })
        }) | ftxui::bgcolor(Theme::Background);
    });
}

void SettingsScreen::setAuthStatus(bool is_authenticated) {
    is_authenticated_ = is_authenticated;
}

ftxui::Component SettingsScreen::GetComponent() {
    return component_;
}

} // namespace ymcli::ui::screens
