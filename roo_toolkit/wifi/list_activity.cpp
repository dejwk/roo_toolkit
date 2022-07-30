#include "roo_toolkit/wifi/list_activity.h"

#include "roo_material_icons/filled/24/action.h"
#include "roo_material_icons/outlined/24/navigation.h"
#include "roo_scheduler.h"
#include "roo_smooth_fonts/NotoSans_Condensed/12.h"
#include "roo_smooth_fonts/NotoSans_Condensed/18.h"
#include "roo_smooth_fonts/NotoSans_Condensed/27.h"
#include "roo_smooth_fonts/NotoSans_CondensedBold/18.h"
#include "roo_smooth_fonts/NotoSans_Regular/15.h"
#include "roo_toolkit/wifi/wifi.h"
#include "roo_windows/containers/horizontal_layout.h"
#include "roo_windows/containers/list_layout.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/core/activity.h"
#include "roo_windows/core/task.h"
#include "roo_windows/core/widget.h"
#include "roo_windows/indicators/24/wifi.h"
#include "roo_windows/widgets/blank.h"
#include "roo_windows/widgets/divider.h"
#include "roo_windows/widgets/icon_button.h"
#include "roo_windows/widgets/progress_bar.h"
#include "roo_windows/widgets/switch.h"
#include "roo_windows/widgets/text_label.h"

namespace roo_toolkit {
namespace wifi {

WifiListItem::WifiListItem(const roo_windows::Environment& env,
                           NetworkSelectedFn on_click)
    : HorizontalLayout(env),
      icon_(env),
      ssid_(env, "Foo", roo_display::font_NotoSans_Condensed_18(),
            roo_display::HAlign::Left(), roo_display::VAlign::Middle()),
      lock_icon_(env, ic_filled_24_action_lock()),
      on_click_(on_click) {
  setGravity(roo_windows::Gravity(roo_windows::kHorizontalGravityNone,
                                  roo_windows::kVerticalGravityMiddle));
  add(icon_, HorizontalLayout::Params());
  add(ssid_, HorizontalLayout::Params().setWeight(1));
  add(lock_icon_, HorizontalLayout::Params());
  icon_.setConnectionStatus(roo_windows::WifiIndicator::CONNECTED);
}

WifiListItem::WifiListItem(const WifiListItem& other)
    : HorizontalLayout(other),
      icon_(other.icon_),
      ssid_(other.ssid_),
      lock_icon_(other.lock_icon_),
      on_click_(other.on_click_) {
  add(icon_, HorizontalLayout::Params());
  add(ssid_, HorizontalLayout::Params().setWeight(1));
  add(lock_icon_, HorizontalLayout::Params());
}

// Sets this item to show the specified network.
void WifiListItem::set(const WifiModel::Network& network) {
  ssid_.setContent(network.ssid);
  icon_.setWifiSignalStrength(network.rssi);
  lock_icon_.setVisibility(network.open ? INVISIBLE : VISIBLE);
}

WifiListModel::WifiListModel(WifiModel& wifi_model) : wifi_model_(wifi_model) {}

int WifiListModel::elementCount() {
  return wifi_model_.otherScannedNetworksCount();
}

void WifiListModel::set(int idx, WifiListItem& dest) {
  dest.set(wifi_model_.otherNetwork(idx));
}

Enable::Enable(const roo_windows::Environment& env, WifiModel& model)
    : HorizontalLayout(env),
      model_(model),
      gap_(env, roo_windows::Dimensions(24, 24)),
      label_(env, "Enable Wi-Fi", roo_display::font_NotoSans_Condensed_18(),
             roo_display::HAlign::Left(), roo_display::VAlign::Middle()),
      switch_(env) {
  setGravity(roo_windows::Gravity(roo_windows::kHorizontalGravityNone,
                                  roo_windows::kVerticalGravityMiddle));
  setPadding(roo_windows::Padding(0, -4));
  add(gap_, roo_windows::HorizontalLayout::Params());
  add(label_, roo_windows::HorizontalLayout::Params().setWeight(1));
  add(switch_, roo_windows::HorizontalLayout::Params());
  enabled_color_ = env.theme().color.secondary;
  disabled_color_.set_a(0xC0);
  disabled_color_ = env.theme().color.onSurface;
  disabled_color_.set_a(0x40);
  switch_.setOnClicked([&]() { model_.toggleEnabled(); });
  setBackground(disabled_color_);
}

void Enable::onEnableChanged(bool enabled) {
  switch_.setOn(enabled);
  setBackground(enabled ? enabled_color_ : disabled_color_);
}

CurrentNetwork::CurrentNetwork(const roo_windows::Environment& env)
    : HorizontalLayout(env),
      icon_(env),
      ssid_(env, "", roo_display::font_NotoSans_Condensed_18(),
            roo_display::HAlign::Left(), roo_display::VAlign::Bottom()),
      status_(env, "Disconnected", roo_display::font_NotoSans_Condensed_12(),
              roo_display::HAlign::Left(), roo_display::VAlign::Top()),
      ssid_status_(env) {
  setGravity(roo_windows::Gravity(roo_windows::kHorizontalGravityNone,
                                  roo_windows::kVerticalGravityMiddle));
  add(icon_, HorizontalLayout::Params());
  ssid_status_.add(ssid_, roo_windows::VerticalLayout::Params());
  ssid_status_.add(status_, roo_windows::VerticalLayout::Params());
  add(ssid_status_, HorizontalLayout::Params().setWeight(1));
  icon_.setConnectionStatus(roo_windows::WifiIndicator::DISCONNECTED);
}

void CurrentNetwork::onChange(const WifiModel& model) {
  const WifiModel::Network& current = model.currentNetwork();
  icon_.setWifiSignalStrength(current.rssi);
  ssid_.setContent(current.ssid);
  switch (model.currentNetworkStatus()) {
    case WL_CONNECTED: {
      icon_.setConnectionStatus(roo_windows::WifiIndicator::CONNECTED);
      break;
    }
    case WL_IDLE_STATUS: {
      icon_.setConnectionStatus(
          roo_windows::WifiIndicator::CONNECTED_NO_INTERNET);
      break;
    }
    default: {
      icon_.setConnectionStatus(roo_windows::WifiIndicator::DISCONNECTED);
      break;
    }
  }
}

ListActivityContents::ListActivityContents(
    const roo_windows::Environment& env, WifiModel& wifi_model,
    NetworkSelectedFn network_selected_fn)
    : VerticalLayout(env),
      wifi_model_(wifi_model),
      title_(env, "WiFi"),
      enable_(env, wifi_model),
      progress_(env),
      current_(env),
      divider_(env),
      list_model_(wifi_model),
      list_(env, list_model_, WifiListItem(env, network_selected_fn)) {
  add(title_, VerticalLayout::Params());
  add(enable_, VerticalLayout::Params());
  add(progress_, VerticalLayout::Params());
  add(current_, VerticalLayout::Params());
  add(divider_, VerticalLayout::Params());
  add(list_, VerticalLayout::Params());
  current_.setVisibility(GONE);
  divider_.setVisibility(GONE);
  progress_.setColor(env.theme().color.secondary);
  progress_.setVisibility(INVISIBLE);
}

void ListActivityContents::onEnableChanged(bool enabled) {
  bool hasNetwork = !wifi_model_.currentNetwork().ssid.empty();
  current_.setVisibility(enabled && hasNetwork ? VISIBLE : GONE);
  divider_.setVisibility(enabled && hasNetwork ? VISIBLE : GONE);
  list_.setVisibility(enabled ? VISIBLE : GONE);
  if (!enabled) progress_.setVisibility(INVISIBLE);
  enable_.onEnableChanged(enabled);
}

void ListActivityContents::onScanStarted() { progress_.setVisibility(VISIBLE); }

void ListActivityContents::onScanCompleted() {
  progress_.setVisibility(INVISIBLE);
  LOG(INFO) << "Called list updated";
  // list_model_.refresh();
  LOG(INFO) << "Model now has " << list_model_.elementCount() << " items";
  list_.modelChanged();
}

void ListActivityContents::onCurrentNetworkChanged() {
  LOG(INFO) << "Current network changed to "
            << wifi_model_.currentNetwork().ssid;
  if (wifi_model_.currentNetwork().ssid.empty()) {
    current_.setVisibility(GONE);
    divider_.setVisibility(GONE);
  } else {
    current_.setVisibility(VISIBLE);
    divider_.setVisibility(VISIBLE);
  }
  current_.onChange(wifi_model_);
}

ListActivity::ListActivity(const roo_windows::Environment& env,
                           WifiModel& wifi_model,
                           roo_scheduler::Scheduler& scheduler,
                           NetworkSelectedFn network_selected_fn)
    : wifi_model_(wifi_model),
      contents_(env, wifi_model, network_selected_fn),
      scrollable_container_(env, contents_),
      start_scan_(&scheduler, [this]() { startScan(); }) {}

void ListActivity::onStart() { onEnableChanged(wifi_model_.isEnabled()); }

void ListActivity::onEnableChanged(bool enabled) {
  contents_.onEnableChanged(enabled);
  if (enabled) {
    if (wifi_model_.isScanCompleted()) {
      contents_.onScanCompleted();
      start_scan_.scheduleAfter(roo_time::Seconds(15));
    } else {
      startScan();
    }
  } else {
    start_scan_.cancel();
  }
}

void ListActivity::onStop() { start_scan_.cancel(); }

void ListActivity::startScan() {
  if (wifi_model_.startScan()) {
    contents_.onScanStarted();
  }
}

void ListActivity::onScanCompleted() {
  contents_.onScanCompleted();
  if (wifi_model_.isEnabled()) {
    start_scan_.scheduleAfter(roo_time::Seconds(15));
  }
}

void ListActivity::onCurrentNetworkChanged() {
  contents_.onCurrentNetworkChanged();
}

void ListActivity::onConnectionStateChanged(Interface::EventType type) {
  contents_.onConnectionStateChanged(type);
}

}  // namespace wifi
}  // namespace roo_toolkit
