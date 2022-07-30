#pragma once

#include "roo_windows/containers/horizontal_layout.h"
#include "roo_windows/widgets/icon_button.h"
#include "roo_windows/widgets/text_label.h"

namespace roo_toolkit {
namespace wifi {

// The title of the activity, with the back button.
class ActivityTitle : public roo_windows::HorizontalLayout {
 public:
  ActivityTitle(const roo_windows::Environment& env, const std::string& title);

  void setTitle(const std::string& ssid) { label_.setContent(ssid); }

 private:
  roo_windows::IconButton back_;
  roo_windows::TextLabel label_;
};

}  // namespace wifi
}  // namespace roo_toolkit
