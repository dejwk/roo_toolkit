#pragma once

#include "roo_display/core/utf8.h"
#include "roo_toolkit/menu/title.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/core/activity.h"

namespace roo_toolkit {
namespace menu {

// To use this class to build your own menus, subclass it, add child widgets,
// and overrice the constructor to add those widgets (calling add(), using
// vertical layout options if needed).
class Menu : public roo_windows::Activity, public roo_windows::VerticalLayout {
 public:
  Menu(const roo_windows::Environment& env, std::string title)
      : roo_windows::Activity(),
        roo_windows::VerticalLayout(env),
        title_(env, std::move(title)) {
    add(title_);
  }

  Widget& getContents() override { return *this; }

 private:
  Title title_;
};

}  // namespace menu
}  // namespace roo_toolkit
