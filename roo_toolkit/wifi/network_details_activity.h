#pragma once

#include "roo_smooth_fonts/NotoSans_Condensed/15.h"
#include "roo_smooth_fonts/NotoSans_Condensed/18.h"
#include "roo_smooth_fonts/NotoSans_Condensed/27.h"
#include "roo_smooth_fonts/NotoSans_CondensedBold/18.h"
#include "roo_smooth_fonts/NotoSans_Regular/15.h"
#include "roo_smooth_fonts/NotoSans_Regular/18.h"
#include "roo_toolkit/wifi/activity_title.h"
#include "roo_toolkit/wifi/model.h"
#include "roo_toolkit/wifi/resolved_interface.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/core/activity.h"
#include "roo_windows/core/task.h"
#include "roo_windows/widgets/divider.h"
#include "roo_windows/widgets/icon_button.h"
#include "roo_windows/widgets/text_field.h"
#include "roo_windows/indicators/36/wifi.h"

namespace roo_toolkit {
namespace wifi {

// All of the widgets of the 'enter password' activity.
class NetworkDetailsActivityContents : public roo_windows::VerticalLayout {
 public:
  NetworkDetailsActivityContents(const roo_windows::Environment& env)
      : roo_windows::VerticalLayout(env),
        title_(env, "WiFi connection details"),
        indicator_(env),
        ssid_(env, "", roo_display::font_NotoSans_Condensed_18(),
              roo_display::kCenter | roo_display::kMiddle),
        status_(env, "", roo_display::font_NotoSans_Condensed_15(),
                roo_display::kCenter | roo_display::kMiddle),
        d1_(env) {
    setGravity(roo_windows::Gravity(roo_windows::kHorizontalGravityCenter,
                                    roo_windows::kVerticalGravityMiddle));
    add(title_, VerticalLayout::Params().setGravity(roo_windows::kHorizontalGravityLeft));
    add(indicator_, VerticalLayout::Params().setVerticalPadding(roo_windows::PADDING_TINY));
    add(ssid_, VerticalLayout::Params().setVerticalPadding(roo_windows::PADDING_NONE));
    add(status_, VerticalLayout::Params().setVerticalPadding(roo_windows::PADDING_NONE));
    add(d1_, VerticalLayout::Params().setWeight(1));
    indicator_.setConnectionStatus(roo_windows::WifiIndicator::DISCONNECTED);
  }

  void enter(const std::string& ssid) { ssid_.setContent(ssid); }

  void onDetailsChanged(int16_t rssi, ConnectionStatus status, bool connecting) {
    indicator_.setWifiSignalStrength(rssi);
    switch (status) {
      case WL_CONNECTED: {
        indicator_.setConnectionStatus(roo_windows::WifiIndicator::CONNECTED);
        break;
      }
      case WL_IDLE_STATUS: {
        indicator_.setConnectionStatus(
            roo_windows::WifiIndicator::CONNECTED_NO_INTERNET);
        break;
      }
      default: {
        indicator_.setConnectionStatus(roo_windows::WifiIndicator::DISCONNECTED);
        break;
      }
    }

    status_.setContent(StatusAsString(status, connecting));
  }

 private:
  ActivityTitle title_;
  roo_windows::WifiIndicator36x36 indicator_;
  roo_windows::TextLabel ssid_;
  roo_windows::TextLabel status_;
  roo_windows::HorizontalDivider d1_;
};

class NetworkDetailsActivity : public roo_windows::Activity {
 public:
  NetworkDetailsActivity(const roo_windows::Environment& env,
                         WifiModel& wifi_model)
      : roo_windows::Activity(),
        wifi_model_(wifi_model),
        ssid_(),
        contents_(env),
        scrollable_container_(env, contents_) {}

  roo_windows::Widget& getContents() override { return scrollable_container_; }

  void enter(roo_windows::Task& task, const std::string& ssid) {
    task.enterActivity(this);
    ssid_ = ssid;
    contents_.enter(ssid_);
    onScanCompleted();
    onCurrentNetworkChanged();
  }

  void onStop() override { ssid_ = ""; }

  void onCurrentNetworkChanged() {
    if (ssid_.empty()) return;  // Not active.
    if (ssid_ != wifi_model_.currentNetwork().ssid) return;
    contents_.onDetailsChanged(wifi_model_.currentNetwork().rssi,
                               wifi_model_.currentNetworkStatus(),
                               wifi_model_.isConnecting());
  }

  void onScanCompleted() {
    if (ssid_.empty()) return;  // Not active.
    const WifiModel::Network* net = wifi_model_.lookupNetwork(ssid_);
    if (net == nullptr) {
      // Out network is no longer in range.
      contents_.onDetailsChanged(-128, WL_NO_SSID_AVAIL, false);
    } else if (net->ssid == wifi_model_.currentNetwork().ssid) {
      // No change. Our network is still current, and in range.
    } else {
      // Our network is no longer current.
      contents_.onDetailsChanged(net->rssi, WL_DISCONNECTED, false);
    }
  }

 private:
  std::string ssid_;
  WifiModel& wifi_model_;
  NetworkDetailsActivityContents contents_;
  roo_windows::ScrollablePanel scrollable_container_;
};

}  // namespace wifi
}  // namespace roo_toolkit
