#include "roo_toolkit/wifi/enter_password_activity.h"

namespace roo_toolkit {
namespace wifi {

EditedPassword::EditedPassword(const roo_windows::Environment& env,
                               roo_windows::TextFieldEditor& editor,
                               std::function<void()> confirm_fn)
    : TextField(env, editor, roo_display::font_NotoSans_Regular_18(),
                "enter password", roo_display::kLeft | roo_display::kMiddle,
                UNDERLINE),
      confirm_fn_(confirm_fn) {}

void EditedPassword::onEditFinished(bool confirmed) {
  Serial.printf("EDIT FINISHED: %d\n", confirmed);
  TextField::onEditFinished(confirmed);
  if (confirmed) {
    // Triggered by a direct click on the 'Enter' button.
    confirm_fn_();
  }
}

EnterPasswordActivity::EnterPasswordActivity(
    const roo_windows::Environment& env, roo_windows::TextFieldEditor& editor,
    WifiModel& wifi_model)
    : roo_windows::Activity(),
      wifi_model_(wifi_model),
      ssid_(nullptr),
      editor_(editor),
      contents_(env, editor, [this]() { confirm(); }) {}

void EnterPasswordActivity::confirm() {
  Serial.println("CONFIRMED!");
  editor_.edit(nullptr);
  wifi_model_.setPassword(*ssid_, passwd());
  wifi_model_.connect(*ssid_, passwd());
  exit();
  //     // if (!editing_) return;
  //     // editing_ = false;
  //     text_.editor().edit(nullptr);
  //     // enter_fn_(text_.content());
  //     // enter_fn_ = nullptr;
  //     exit();
}

}  // namespace wifi
}  // namespace roo_toolkit