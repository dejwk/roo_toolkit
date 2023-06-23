#pragma once

#include "roo_display/core/utf8.h"
#include "roo_display/image/image.h"
#include "roo_windows/core/activity.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/widgets/icon.h"

// Clickable option with single-line text label and an optional icon.
// Note: the text label is expected to be a constant.

namespace roo_toolkit {
namespace menu {

class BasicNavigationItem : public roo_windows::HorizontalLayout {
 public:
  BasicNavigationItem(const roo_windows::Environment& env,
                      const roo_display::Pictogram& icon,
                      roo_display::StringView text,
                      roo_windows::Activity& target);

  roo_windows::PreferredSize getPreferredSize() const override {
    return roo_windows::PreferredSize(
        roo_windows::PreferredSize::MatchParentWidth(),
        roo_windows::PreferredSize::WrapContentHeight());
  }

 private:
  roo_windows::Icon icon_;
  roo_windows::StringViewLabel label_;
  roo_windows::Activity& target_;
};

}  // namespace menu
}  // namespace roo_toolkit
