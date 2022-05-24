#pragma once

// #include <Arduino.h>

#include "roo_scheduler.h"

#include "roo_smooth_fonts/NotoSans_Condensed/18.h"
#include "roo_windows/containers/horizontal_layout.h"
#include "roo_windows/containers/list_layout.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/core/activity.h"
#include "roo_windows/core/widget.h"
#include "roo_windows/indicators/24/wifi.h"
#include "roo_windows/widgets/divider.h"
#include "roo_windows/widgets/text_label.h"

#include "roo_toolkit/wifi/interface.h"
#include "roo_toolkit/wifi/network_details.h"

namespace roo_toolkit {
namespace wifi {

class WifiListItem : public roo_windows::HorizontalLayout {
 public:
  WifiListItem(const roo_windows::Environment& env)
      : HorizontalLayout(env),
        icon_(env),
        ssid_(env, "", roo_display::font_NotoSans_Condensed_18(),
              roo_display::HAlign::Left(), roo_display::VAlign::Middle()) {
    setGravity(roo_windows::Gravity(roo_windows::kHorizontalGravityNone,
                                    roo_windows::kVerticalGravityMiddle));
    add(icon_, HorizontalLayout::Params());
    add(ssid_, HorizontalLayout::Params().setWeight(1));
    icon_.setWifiConnected(true);
  }

  WifiListItem(const WifiListItem& other)
      : HorizontalLayout(other), icon_(other.icon_), ssid_(other.ssid_) {
    add(icon_, HorizontalLayout::Params());
    add(ssid_, HorizontalLayout::Params().setWeight(1));
  }

  bool isClickable() const override { return true; }

  void set(const NetworkDetails& network) {
    ssid_.setContent(std::string((const char*)network.ssid,
                                 strlen((const char*)network.ssid)));
    icon_.setWifiSignalStrength(network.rssi);
  }

  roo_windows::PreferredSize getPreferredSize() const override {
    return roo_windows::PreferredSize(
        roo_windows::PreferredSize::MatchParent(),
        roo_windows::PreferredSize::WrapContent());
  }

 private:
  roo_windows::WifiIndicator24x24 icon_;
  roo_windows::TextLabel ssid_;
};

class WifiListModel : public roo_windows::ListModel<WifiListItem> {
 public:
  void refresh(const Interface& interface) {
    std::vector<NetworkDetails> raw_data;
    interface.getScanResults(&raw_data, 100);
    int raw_count = raw_data.size();
    if (raw_count == 0) {
      networks_.clear();
      return;
    }
    // De-duplicate SSID, keeping the one with the strongest signal.
    // Start by sorting by (ssid, signal strength).
    std::vector<uint8_t> indices(raw_data.size(), 0);
    for (uint8_t i = 0; i < raw_count; ++i) indices[i] = i;
    std::sort(&indices[0], &indices[raw_count], [&](int a, int b) -> bool {
      int ssid_cmp = strncmp((const char*)raw_data[a].ssid, (const char*)raw_data[b].ssid, 33);
      if (ssid_cmp < 0) return true;
      if (ssid_cmp > 0) return false;
      return raw_data[a].rssi > raw_data[b].rssi;
    });
    // Now, compact the result by keeping the first value for each SSID.
    const char* current_ssid = (const char*)raw_data[indices[0]].ssid;
    uint8_t src = 1;
    uint8_t dst = 1;
    while (src < raw_count) {
      const char* candidate_ssid = (const char*)raw_data[indices[src]].ssid;
      if (strncmp(current_ssid, candidate_ssid, 33) != 0) {
        current_ssid = candidate_ssid;
        indices[dst++] = indices[src];
      }
      ++src;
    }
    // Now sort again, this time by signal strength only.
    std::sort(&indices[0], &indices[dst], [&](int a, int b) -> bool {
      return raw_data[a].rssi > raw_data[b].rssi;
    });
    // Finally, copy over the results.
    networks_.resize(dst);
    for (uint8_t i = 0; i < dst; ++i) {
      networks_[i] = std::move(raw_data[indices[i]]);
    }
  }

  int elementCount() override { return networks_.size(); }

  void set(int idx, WifiListItem& dest) { dest.set(networks_[idx]); }

  //   void setChangeListener(std::function<void(int begin, int end)>
  //   change_listener) override {
  //     change_listener_ = change_listener;
  //   }

 private:
  std::vector<NetworkDetails> networks_;
  //   std::function<void(int begin, int end)> change_listener_;
};

class CurrentNetwork : public roo_windows::HorizontalLayout {
 public:
  CurrentNetwork(const roo_windows::Environment& env)
      : HorizontalLayout(env),
        icon_(env),
        ssid_(env, "", roo_display::font_NotoSans_Condensed_18(),
              roo_display::HAlign::Left(), roo_display::VAlign::Middle()) {
    setGravity(roo_windows::Gravity(roo_windows::kHorizontalGravityNone,
                                    roo_windows::kVerticalGravityMiddle));
    add(icon_, HorizontalLayout::Params());
    add(ssid_, HorizontalLayout::Params().setWeight(1));
    icon_.setWifiConnected(true);
  }

  bool isClickable() const override { return true; }

  void set(const NetworkDetails& network) {
    ssid_.setContent(std::string((const char*)network.ssid,
                                 strlen((const char*)network.ssid)));
    icon_.setWifiSignalStrength(network.rssi);
  }

  roo_windows::PreferredSize getPreferredSize() const override {
    return roo_windows::PreferredSize(
        roo_windows::PreferredSize::MatchParent(),
        roo_windows::PreferredSize::WrapContent());
  }

 private:
  roo_windows::WifiIndicator24x24 icon_;
  roo_windows::TextLabel ssid_;
};

class ListActivityContents : public roo_windows::VerticalLayout {
 public:
  ListActivityContents(const roo_windows::Environment& env,
                       Interface& interface)
      : VerticalLayout(env),
        interface_(interface),
        current_(env),
        divider_(env),
        list_model_(),
        list_(env, list_model_, WifiListItem(env)) {
    add(current_, VerticalLayout::Params());
    add(divider_, VerticalLayout::Params());
    add(list_, VerticalLayout::Params());
    NetworkDetails current;
  }

  void listUpdated() {
    LOG(INFO) << "Called list updated";
    list_model_.refresh(interface_);
    LOG(INFO) << "Model now has " << list_model_.elementCount() << " items";
    list_.modelChanged();
  }

  void setCurrent(const NetworkDetails& current) {
    current_.set(current);
  }

 private:
  Interface& interface_;
  CurrentNetwork current_;
  roo_windows::HorizontalDivider divider_;
  WifiListModel list_model_;
  roo_windows::ListLayout<WifiListItem> list_;
};

class ListActivity : public roo_windows::Activity {
 public:
  ListActivity(const roo_windows::Environment& env, Interface& interface,
               roo_scheduler::Scheduler& scheduler)
      : interface_(interface),
        contents_(env, interface),
        scrollable_container_(env, contents_),
        activity_running_(false),
        scheduler_(scheduler),
        start_scan_([this]() { startScan(); }),
        check_scan_completed_([this]() { checkScanCompleted(); }),
        check_pending_(false) {}

  roo_windows::Widget& getContents() override { return scrollable_container_; }

  void onStart() override {
    activity_running_ = true;
    NetworkDetails current;
    interface_.getApInfo(&current);
    contents_.setCurrent(current);
    if (check_pending_) return;
    if (interface_.scanCompleted()) {
      contents_.listUpdated();
      scheduler_.scheduleAfter(&start_scan_, roo_time::Seconds(15));
      check_pending_ = true;
    } else {
      startScan();
    }
  }

  void onStop() override { activity_running_ = false; }

 private:
  void startScan() {
    if (!activity_running_) {
      check_pending_ = false;
      return;
    }
    if (interface_.startScan()) {
      scheduler_.scheduleAfter(&check_scan_completed_, roo_time::Millis(500));
      check_pending_ = true;
    }
  }

  void checkScanCompleted() {
    if (interface_.scanCompleted()) {
      contents_.listUpdated();
      if (!activity_running_) {
        check_pending_ = false;
        return;
      }
      scheduler_.scheduleAfter(&start_scan_, roo_time::Seconds(15));
    } else {
      if (!activity_running_) {
        check_pending_ = false;
        return;
      }
      scheduler_.scheduleAfter(&check_scan_completed_, roo_time::Millis(500));
    }
  }

  Interface& interface_;

  ListActivityContents contents_;
  roo_windows::ScrollablePanel scrollable_container_;

  bool activity_running_;

  roo_scheduler::Scheduler& scheduler_;
  roo_scheduler::Task start_scan_;
  roo_scheduler::Task check_scan_completed_;
  bool check_pending_;
};

}  // namespace wifi
}  // namespace roo_toolkit
