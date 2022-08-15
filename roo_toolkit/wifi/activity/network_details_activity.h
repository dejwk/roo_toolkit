#pragma once

#include "roo_material_icons/filled/24/action.h"
#include "roo_material_icons/filled/24/content.h"
#include "roo_material_icons/filled/24/notification.h"
#include "roo_smooth_fonts/NotoSans_Condensed/15.h"
#include "roo_smooth_fonts/NotoSans_Condensed/18.h"
#include "roo_smooth_fonts/NotoSans_Condensed/27.h"
#include "roo_smooth_fonts/NotoSans_CondensedBold/18.h"
#include "roo_smooth_fonts/NotoSans_Regular/15.h"
#include "roo_smooth_fonts/NotoSans_Regular/18.h"
#include "roo_toolkit/wifi/activity/activity_title.h"
#include "roo_toolkit/wifi/device/resolved_interface.h"
#include "roo_windows/containers/horizontal_layout.h"
#include "roo_windows/containers/stacked_layout.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/core/activity.h"
#include "roo_windows/core/task.h"
#include "roo_windows/indicators/36/wifi.h"
#include "roo_windows/widgets/divider.h"
#include "roo_windows/widgets/icon.h"
#include "roo_windows/widgets/icon_with_caption.h"
#include "roo_windows/widgets/text_field.h"

namespace roo_toolkit {
namespace wifi {

typedef std::function<void(roo_windows::Task& task, const std::string& ssid)>
    DetailsEditedFn;

// All of the widgets of the 'enter password' activity.
class NetworkDetailsActivityContents : public roo_windows::VerticalLayout {
 public:
  NetworkDetailsActivityContents(const roo_windows::Environment& env,
                                 Controller& model,
                                 std::function<void()> edit_fn)
      : roo_windows::VerticalLayout(env),
        wifi_model_(model),
        title_(env, "Network details"),
        edit_(env, ic_filled_24_content_create()),
        indicator_(env),
        ssid_(env, "", roo_display::font_NotoSans_Condensed_18(),
              roo_display::kCenter | roo_display::kMiddle),
        status_(env, "", roo_display::font_NotoSans_Condensed_15(),
                roo_display::kCenter | roo_display::kMiddle),
        d1_(env),
        actions_(env),
        button_forget_(env, ic_filled_24_action_delete(), "Forget"),
        button_connect_(env, ic_filled_24_notification_wifi(), "Connect") {
    setGravity(roo_windows::Gravity(roo_windows::kHorizontalGravityCenter,
                                    roo_windows::kVerticalGravityMiddle));
    edit_.setOnClicked(edit_fn);
    title_.add(edit_, roo_windows::HorizontalLayout::Params());
    add(title_, VerticalLayout::Params().setGravity(
                    roo_windows::kHorizontalGravityLeft));
    indicator_.setPadding(roo_windows::PADDING_TINY);
    add(indicator_, VerticalLayout::Params());
    ssid_.setPadding(roo_windows::PADDING_NONE);
    ssid_.setMargins(roo_windows::MARGIN_NONE);
    status_.setPadding(roo_windows::PADDING_NONE);
    status_.setMargins(roo_windows::MARGIN_NONE);
    add(ssid_, VerticalLayout::Params());
    add(status_, VerticalLayout::Params());
    add(d1_, VerticalLayout::Params().setWeight(1));
    indicator_.setConnectionStatus(roo_windows::WifiIndicator::DISCONNECTED);
    actions_.setUseLargestChild(true);
    button_forget_.setPadding(roo_windows::PADDING_HUGE);
    button_connect_.setPadding(roo_windows::PADDING_HUGE);
    roo_display::Color pri = env.theme().color.primary;
    button_forget_.setColor(pri);
    button_connect_.setColor(pri);
    actions_.add(button_forget_,
                 roo_windows::HorizontalLayout::Params().setWeight(1));
    actions_.add(button_connect_,
                 roo_windows::HorizontalLayout::Params().setWeight(1));

    add(actions_, VerticalLayout::Params());
  }

  void enter(const std::string& ssid) { ssid_.setContent(ssid); }

  void onDetailsChanged(int16_t rssi, ConnectionStatus status,
                        bool connecting) {
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
        indicator_.setConnectionStatus(
            roo_windows::WifiIndicator::DISCONNECTED);
        break;
      }
    }
    status_.setContent(StatusAsString(status, connecting));
    if (status == WL_CONNECTED) {
      button_connect_.setCaption("Disconnect");
      button_connect_.setIcon(ic_filled_24_content_clear());
      button_connect_.setColor(theme().color.primary);
      button_connect_.setOnClicked([this]() { disconnect(); });
    } else if (connecting) {
      button_connect_.setCaption("Connecting...");
      button_connect_.setIcon(ic_filled_24_content_clear());
      roo_display::Color disabled = theme().color.onSurface;
      disabled.set_a(0x20);
      button_connect_.setColor(disabled);
      button_connect_.setOnClicked(nullptr);
    } else {
      button_connect_.setCaption("Connect");
      button_connect_.setIcon(ic_filled_24_notification_wifi());
      button_connect_.setColor(theme().color.primary);
      button_connect_.setOnClicked([this]() { connect(); });
    }
  }

 private:
  void connect() {
    wifi_model_.connect();
  }

  void disconnect() {
    wifi_model_.disconnect();
  }

  Controller& wifi_model_;
  ActivityTitle title_;
  roo_windows::WifiIndicator36x36 indicator_;
  roo_windows::Icon edit_;
  roo_windows::TextLabel ssid_;
  roo_windows::TextLabel status_;
  roo_windows::HorizontalDivider d1_;
  roo_windows::HorizontalLayout actions_;
  roo_windows::IconWithCaption button_forget_;
  roo_windows::IconWithCaption button_connect_;
};

class NetworkDetailsActivity : public roo_windows::Activity {
 public:
  NetworkDetailsActivity(const roo_windows::Environment& env,
                         Controller& wifi_model, DetailsEditedFn edit_fn)
      : roo_windows::Activity(),
        wifi_model_(wifi_model),
        ssid_(),
        contents_(
            env, wifi_model,
            [this, edit_fn]() { edit_fn(*getContents().getTask(), ssid_); }),
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
    const Controller::Network* net = wifi_model_.lookupNetwork(ssid_);
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
  Controller& wifi_model_;
  NetworkDetailsActivityContents contents_;
  roo_windows::ScrollablePanel scrollable_container_;
};

}  // namespace wifi
}  // namespace roo_toolkit
