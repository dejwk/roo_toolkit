#pragma once

#include <Arduino.h>

#include "roo_scheduler.h"
#include "roo_toolkit/menu/title.h"
#include "roo_toolkit/wifi/device/resolved_interface.h"
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
#include "roo_windows/widgets/text_label.h"

namespace roo_toolkit {
namespace wifi {

class WifiSetup;

typedef std::function<void(roo_windows::Task& task, const std::string& ssid)>
    NetworkSelectedFn;

// Single WiFi network in a list.
class WifiListItem : public roo_windows::HorizontalLayout {
 public:
  WifiListItem(const roo_windows::Environment& env, NetworkSelectedFn on_click);

  WifiListItem(const WifiListItem& other);

  bool isClickable() const override { return true; }

  // Sets this item to show the specified network.
  void set(const Controller::Network& network);

  roo_windows::PreferredSize getPreferredSize() const override {
    return roo_windows::PreferredSize(
        roo_windows::PreferredSize::MatchParentWidth(),
        roo_windows::PreferredSize::WrapContentHeight());
  }

  void onClicked() override { on_click_(*getTask(), ssid_.content()); }

 private:
  bool isOpen() const { return is_open_; }

  roo_windows::WifiIndicator icon_;
  roo_windows::TextLabel ssid_;
  bool is_open_;
  roo_windows::Icon lock_icon_;
  NetworkSelectedFn on_click_;
};

class WifiListModel : public roo_windows::ListModel<WifiListItem> {
 public:
  WifiListModel(Controller& wifi_model);

  int elementCount() override;
  void set(int idx, WifiListItem& dest);

 private:
  Controller& wifi_model_;
};

// The list of WiFi networks.
class WifiList : public roo_windows::ListLayout<WifiListItem> {
 public:
  using roo_windows::ListLayout<WifiListItem>::ListLayout;
};

// The main 'enable WiFi' bar.
class Enable : public roo_windows::HorizontalLayout {
 public:
  Enable(const roo_windows::Environment& env, Controller& model);

  roo_windows::PreferredSize getPreferredSize() const override {
    return roo_windows::PreferredSize(
        roo_windows::PreferredSize::MatchParentWidth(),
        roo_windows::PreferredSize::WrapContentHeight());
  }

  bool isClickable() const override { return true; }

  void onClicked() override { model_.toggleEnabled(); }

  void onEnableChanged(bool enabled);

 private:
  Controller& model_;

  roo_windows::Blank gap_;
  roo_windows::TextLabel label_;
  roo_windows::Switch switch_;

  roo_display::Color enabled_color_;
  roo_display::Color disabled_color_;
};

// Shows the currently selected network.
class CurrentNetwork : public roo_windows::HorizontalLayout {
 public:
  CurrentNetwork(const roo_windows::Environment& env,
                 NetworkSelectedFn on_click);

  bool isClickable() const override { return true; }

  void onClicked() override { on_click_(*getTask(), ssid_.content()); }

  void onChange(const Controller& model);

  roo_windows::PreferredSize getPreferredSize() const override {
    return roo_windows::PreferredSize(
        roo_windows::PreferredSize::MatchParentWidth(),
        roo_windows::PreferredSize::WrapContentHeight());
  }

 private:
  roo_windows::WifiIndicator indicator_;
  roo_windows::TextLabel ssid_;
  roo_windows::TextLabel status_;
  roo_windows::VerticalLayout ssid_status_;
  NetworkSelectedFn on_click_;
};

// All of the widgets of the list activity.
class ListActivityContents : public roo_windows::VerticalLayout {
 public:
  ListActivityContents(const roo_windows::Environment& env,
                       Controller& wifi_model,
                       NetworkSelectedFn network_selected_fn);

  void onEnableChanged(bool enabled);

  void onScanStarted();
  void onScanCompleted();
  void onCurrentNetworkChanged();

  void onConnectionStateChanged(Interface::EventType type) {
    current_.onChange(wifi_model_);
  }

 private:
  Controller& wifi_model_;
  roo_toolkit::menu::Title title_;
  Enable enable_;
  roo_windows::ProgressBar progress_;
  CurrentNetwork current_;
  roo_windows::HorizontalDivider divider_;
  WifiListModel list_model_;
  WifiList list_;
};

class ListActivity : public roo_windows::Activity {
 public:
  ListActivity(const roo_windows::Environment& env, Controller& wifi_model,
               NetworkSelectedFn network_selected_fn);

  roo_windows::Widget& getContents() override { return scrollable_container_; }

  void onStart() override;
  void onStop() override;

 private:
  friend class WifiSetup;

  void startScan();

  void onEnableChanged(bool enabled);
  void onScanStarted();
  void onScanCompleted();
  void onCurrentNetworkChanged();
  void onConnectionStateChanged(Interface::EventType type);

  Controller& wifi_model_;

  ListActivityContents contents_;
  roo_windows::ScrollablePanel scrollable_container_;
};

}  // namespace wifi
}  // namespace roo_toolkit
