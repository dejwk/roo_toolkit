#pragma once

#include "roo_material_icons/outlined/24/navigation.h"
#include "roo_smooth_fonts/NotoSans_Regular/18.h"
#include "roo_toolkit/wifi/activity_title.h"
#include "roo_toolkit/wifi/model.h"
#include "roo_toolkit/wifi/resolved_interface.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/core/activity.h"
#include "roo_windows/core/task.h"
#include "roo_windows/widgets/icon_button.h"
#include "roo_windows/widgets/text_field.h"

namespace roo_toolkit {
namespace wifi {

class EnterPasswordActivity;

class EditedPassword : public roo_windows::TextField {
 public:
  EditedPassword(const roo_windows::Environment& env,
                 roo_windows::TextFieldEditor& editor,
                 std::function<void()> confirm_fn);

  void onEditFinished(bool confirmed) override;

 private:
  std::function<void()> confirm_fn_;
};

class PasswordBar : public roo_windows::HorizontalLayout {
 public:
  PasswordBar(const roo_windows::Environment& env,
              roo_windows::TextFieldEditor& editor,
              std::function<void()> confirm_fn)
      : roo_windows::HorizontalLayout(env),
        visibility_(env),
        text_(env, editor, confirm_fn),
        enter_(env, ic_outlined_24_navigation_check()) {
    text_.setStarred(true);
    visibility_.setOff();
    visibility_.setOnClicked([this]() { visibilityChanged(); });
    setGravity(roo_windows::Gravity(roo_windows::kHorizontalGravityNone,
                                    roo_windows::kVerticalGravityMiddle));
    add(visibility_, HorizontalLayout::Params());
    add(text_, HorizontalLayout::Params().setWeight(1));
    add(enter_, HorizontalLayout::Params());
    enter_.setOnClicked(confirm_fn);
  }

  void edit() { text_.edit(); }
  void clear() { text_.setContent(""); }

  const std::string& passwd() const { return text_.content(); }

 private:
  void visibilityChanged() { text_.setStarred(visibility_.isOff()); }

  roo_windows::VisibilityToggle visibility_;
  EditedPassword text_;
  roo_windows::Button enter_;
};

// All of the widgets of the 'enter password' activity.
class EnterPasswordActivityContents : public roo_windows::VerticalLayout {
 public:
  EnterPasswordActivityContents(const roo_windows::Environment& env,
                                roo_windows::TextFieldEditor& editor,
                                std::function<void()> confirm_fn)
      : roo_windows::VerticalLayout(env),
        title_(env, ""),
        pwbar_(env, editor, confirm_fn) {
    add(title_, VerticalLayout::Params());
    add(pwbar_, VerticalLayout::Params());
  }

  void enter(const std::string& ssid) {
    title_.setTitle(ssid);
    pwbar_.edit();
  }

  void clear() { pwbar_.clear(); }

  const std::string& passwd() const { return pwbar_.passwd(); }

 private:
  // void confirm() { getTask()->exitActivity(); }

  // Wifi& wifi_;

  ActivityTitle title_;
  PasswordBar pwbar_;
};

class EnterPasswordActivity : public roo_windows::Activity {
 public:
  EnterPasswordActivity(const roo_windows::Environment& env,
                        roo_windows::TextFieldEditor& editor,
                        WifiModel& wifi_model);

  roo_windows::Widget& getContents() override { return contents_; }

  void enter(const std::string& ssid) {
    ssid_ = &ssid;
    contents_.enter(ssid);
  }

  void onPause() override { editor_.edit(nullptr); }
  void onStop() override { contents_.clear(); }

 private:
  const std::string& passwd() const { return contents_.passwd(); }

  void confirm();

  WifiModel& wifi_model_;
  const std::string* ssid_;
  roo_windows::TextFieldEditor& editor_;
  EnterPasswordActivityContents contents_;
};

}  // namespace wifi
}  // namespace roo_toolkit
