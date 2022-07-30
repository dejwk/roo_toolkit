#include "roo_toolkit/wifi/activity_title.h"

#include "roo_windows/core/task.h"
#include "roo_material_icons/outlined/24/navigation.h"
#include "roo_smooth_fonts/NotoSans_CondensedBold/18.h"

namespace roo_toolkit {
namespace wifi {

ActivityTitle::ActivityTitle(const roo_windows::Environment& env,
                             const std::string& title)
    : HorizontalLayout(env),
      back_(env, ic_outlined_24_navigation_arrow_back()),
      label_(env, "Wi-Fi", roo_display::font_NotoSans_CondensedBold_18(),
             roo_display::HAlign::Left(), roo_display::VAlign::Middle()) {
  setGravity(roo_windows::Gravity(roo_windows::kHorizontalGravityNone,
                                  roo_windows::kVerticalGravityMiddle));
  add(back_, roo_windows::HorizontalLayout::Params());
  add(label_, roo_windows::HorizontalLayout::Params().setWeight(1));
  back_.setOnClicked([&]() { getTask()->exitActivity(); });
}

}  // namespace wifi
}  // namespace roo_toolkit
