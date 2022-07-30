#pragma once

#include "roo_material_icons/filled/36/device.h"
#include "roo_smooth_fonts/NotoSans_Condensed/15.h"
#include "roo_smooth_fonts/NotoSans_Condensed/18.h"
#include "roo_smooth_fonts/NotoSans_Condensed/27.h"
#include "roo_smooth_fonts/NotoSans_CondensedBold/18.h"
#include "roo_smooth_fonts/NotoSans_Regular/15.h"
#include "roo_smooth_fonts/NotoSans_Regular/18.h"
#include "roo_toolkit/wifi/activity_title.h"
#include "roo_toolkit/wifi/model.h"
#include "roo_toolkit/wifi/resolved_interface.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/core/activity.h"
#include "roo_windows/core/task.h"
#include "roo_windows/widgets/divider.h"
#include "roo_windows/widgets/icon_button.h"
#include "roo_windows/widgets/text_field.h"

namespace roo_toolkit {
namespace wifi {

// class EnterPasswordActivity;

// class EditedPassword : public roo_windows::TextField {
//  public:
//   EditedPassword(const roo_windows::Environment& env,
//                  roo_windows::TextFieldEditor& editor,
//                  std::function<void()> confirm_fn);

//   void onEditFinished(bool confirmed) override;

//  private:
//   std::function<void()> confirm_fn_;
// };

// All of the widgets of the 'enter password' activity.
class NetworkDetailsActivityContents : public roo_windows::VerticalLayout {
 public:
  NetworkDetailsActivityContents(const roo_windows::Environment& env)
      : roo_windows::VerticalLayout(env),
        // wifi_(wifi),
        wifi_icon_(env, ic_filled_36_device_signal_wifi_4_bar()),
        ssid_(env, "", roo_display::font_NotoSans_Condensed_18(),
              roo_display::HAlign::Center(), roo_display::VAlign::Middle()),
        status_(env, "", roo_display::font_NotoSans_Condensed_15(),
                roo_display::HAlign::Center(), roo_display::VAlign::Middle()),
        d1_(env) {
    setGravity(roo_windows::Gravity(roo_windows::kHorizontalGravityCenter,
                                    roo_windows::kVerticalGravityMiddle));
    add(wifi_icon_, VerticalLayout::Params());
    add(ssid_, VerticalLayout::Params());
    add(status_, VerticalLayout::Params());
    add(d1_, VerticalLayout::Params().setWeight(1));
  }

  void enter(const std::string& ssid) { ssid_.setContent(ssid); }

 private:
  // Wifi& wifi_;

  roo_windows::Icon wifi_icon_;
  roo_windows::TextLabel ssid_;
  roo_windows::TextLabel status_;
  roo_windows::HorizontalDivider d1_;

  //   ActivityTitle title_;
  //   PasswordBar pwbar_;
};

class NetworkDetailsActivity : public roo_windows::Activity {
 public:
  NetworkDetailsActivity(const roo_windows::Environment& env, WifiModel& wifi_model)
      : roo_windows::Activity(),
        wifi_model_(wifi_model),
        ssid_(nullptr),
        contents_(env),
        scrollable_container_(env, contents_) {}

  roo_windows::Widget& getContents() override { return scrollable_container_; }

  void enter(roo_windows::Task& task, const std::string& ssid) {
    task.enterActivity(this);
    ssid_ = &ssid;
    contents_.enter(*ssid_);
  }

  //   void onPause() override { editor_.edit(nullptr); }

 private:
  //   const std::string& passwd() const { return contents_.passwd(); }

  //   void confirm() {
  //     Serial.println("CONFIRMED!");
  //     editor_.edit(nullptr);
  //     wifi_.store().setPassword(*ssid_, passwd());
  //     wifi_.connect(*ssid_, passwd());
  //     exit();
  //     //     // if (!editing_) return;
  //     //     // editing_ = false;
  //     //     text_.editor().edit(nullptr);
  //     //     // enter_fn_(text_.content());
  //     //     // enter_fn_ = nullptr;
  //     //     exit();
  //   }

  const std::string* ssid_;
  WifiModel& wifi_model_;
  NetworkDetailsActivityContents contents_;
  roo_windows::ScrollablePanel scrollable_container_;
};

}  // namespace wifi
}  // namespace roo_toolkit
