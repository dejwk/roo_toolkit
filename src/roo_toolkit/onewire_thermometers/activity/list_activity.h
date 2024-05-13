#pragma once

#include <Arduino.h>

#include "roo_scheduler.h"
#include "roo_toolkit/menu/title.h"
#include "roo_windows/containers/horizontal_layout.h"
#include "roo_windows/containers/list_layout.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/core/activity.h"
#include "roo_windows/indicators/wifi.h"
#include "roo_windows/widgets/blank.h"
#include "roo_windows/widgets/divider.h"
#include "roo_windows/widgets/icon.h"
#include "roo_windows/widgets/progress_bar.h"
#include "roo_windows/widgets/switch.h"
// #include "roo_windows/widgets/text_block.h"
#include "roo_onewire/thermometer_roles.h"
#include "roo_windows/widgets/text_label.h"

namespace roo_toolkit {
namespace onewire_thermometers {

typedef std::function<void(roo_windows::Task& task, int id)>
    ThermometerSelectedFn;

class ThermometerListItem : public roo_windows::HorizontalLayout {
 public:
  ThermometerListItem(const roo_windows::Environment& env,
                      ThermometerSelectedFn on_click);

  ThermometerListItem(const ThermometerListItem& other);

  bool isClickable() const override { return true; }

  void set(const roo_onewire::ThermometerRole& thermometer);

  roo_windows::PreferredSize getPreferredSize() const override {
    return roo_windows::PreferredSize(
        roo_windows::PreferredSize::MatchParentWidth(),
        roo_windows::PreferredSize::WrapContentHeight());
  }

  void onClicked() override { on_click_(*getTask(), id_); }

 private:
  // bool isOpen() const { return is_open_; }

  roo_windows::Icon thermometer_icon_;
  int id_;
  roo_windows::TextLabel name_;
  roo_windows::TextLabel reading_;
  // roo_windows::Icon lock_icon_;
  ThermometerSelectedFn on_click_;
};

class ThermometerListModel
    : public roo_windows::ListModel<ThermometerListItem> {
 public:
  ThermometerListModel(roo_onewire::ThermometerRoles& model);

  int elementCount() const override;
  void set(int idx, ThermometerListItem& dest) const override;

 private:
  roo_onewire::ThermometerRoles& model_;
};

// The list of WiFi networks.
class ThermometerList : public roo_windows::ListLayout<ThermometerListItem> {
 public:
  using roo_windows::ListLayout<ThermometerListItem>::ListLayout;
};

// All of the widgets of the list activity.
class ListActivityContents
    : public roo_windows::VerticalLayout,
      public roo_onewire::ThermometerRoles::EventListener {
 public:
  ListActivityContents(const roo_windows::Environment& env,
                       roo_onewire::ThermometerRoles& model,
                       ThermometerSelectedFn thermometer_selected_fn);

  roo_windows::PreferredSize getPreferredSize() const override {
    return roo_windows::PreferredSize(
        roo_windows::PreferredSize::MatchParentWidth(),
        roo_windows::PreferredSize::WrapContentHeight());
  }

  void update() { list_.modelChanged(); }

 private:
  void conversionCompleted() override;

  roo_onewire::ThermometerRoles& model_;
  roo_toolkit::menu::Title title_;
  ThermometerListModel list_model_;
  ThermometerList list_;
};

class ListActivity : public roo_windows::Activity {
 public:
  ListActivity(const roo_windows::Environment& env,
               roo_scheduler::Scheduler& scheduler,
               roo_onewire::ThermometerRoles& model,
               ThermometerSelectedFn network_selected_fn);

  roo_windows::Widget& getContents() override { return scrollable_container_; }

  void onStart() override;
  void onStop() override;

  void onResume() override { contents_.update(); }

 private:
  roo_onewire::ThermometerRoles& model_;

  ListActivityContents contents_;
  roo_windows::ScrollablePanel scrollable_container_;

  // Used to periodically refresh (discover, convert) thermometers. We keep it
  // here, taking advantage of the fact that the list activity is always at the
  // bottom of all thermometer config activities, so we can use Start/Stop to
  // control the polling.
  roo_scheduler::PeriodicTask refresh_task_;
};

}  // namespace onewire_thermometers
}  // namespace roo_toolkit
