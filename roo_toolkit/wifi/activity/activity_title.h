#pragma once

#include "roo_windows/containers/horizontal_layout.h"
#include "roo_windows/widgets/icon.h"
#include "roo_windows/widgets/text_label.h"

namespace roo_toolkit {
namespace wifi {

// The title of the activity, with the back button.
class ActivityTitle : public roo_windows::HorizontalLayout {
 public:
  ActivityTitle(const roo_windows::Environment& env, const std::string& title);

  roo_windows::PreferredSize getPreferredSize() const override {
    return roo_windows::PreferredSize(
        roo_windows::PreferredSize::MatchParentWidth(),
        roo_windows::PreferredSize::WrapContentHeight());
  }

  void setTitle(roo_display::StringView ssid) { label_.setContent(ssid); }

 private:
  roo_windows::Icon back_;
  roo_windows::TextLabel label_;
};

}  // namespace wifi
}  // namespace roo_toolkit
