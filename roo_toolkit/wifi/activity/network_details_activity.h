#pragma once

#include "roo_windows/config.h"

#include "roo_material_icons/filled/18/action.h"
#include "roo_material_icons/filled/24/action.h"
#include "roo_material_icons/filled/36/action.h"
#include "roo_material_icons/filled/48/action.h"
#include "roo_material_icons/filled/18/content.h"
#include "roo_material_icons/filled/24/content.h"
#include "roo_material_icons/filled/36/content.h"
#include "roo_material_icons/filled/48/content.h"
#include "roo_material_icons/filled/18/notification.h"
#include "roo_material_icons/filled/24/notification.h"
#include "roo_material_icons/filled/36/notification.h"
#include "roo_material_icons/filled/48/notification.h"

#include "roo_toolkit/menu/title.h"
#include "roo_toolkit/wifi/activity/resources.h"
#include "roo_toolkit/wifi/device/resolved_interface.h"
#include "roo_windows/containers/horizontal_layout.h"
#include "roo_windows/containers/stacked_layout.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/core/activity.h"
#include "roo_windows/core/task.h"
#include "roo_windows/indicators/wifi.h"
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
        title_(env, kStrNetworkDetails),
        edit_(env, SCALED_ROO_ICON(filled, content_create)),
        indicator_(env),
        ssid_(env, "", roo_windows::font_subtitle1(),
              roo_display::kCenter | roo_display::kMiddle),
        status_(env, "", roo_windows::font_caption(),
                roo_display::kCenter | roo_display::kMiddle),
        d1_(env),
        actions_(env),
        button_forget_(env, SCALED_ROO_ICON(filled, action_delete), kStrForget),
        button_connect_(env, SCALED_ROO_ICON(filled, notification_wifi),
                        kStrConnect) {
    setGravity(roo_windows::Gravity(roo_windows::kHorizontalGravityCenter,
                                    roo_windows::kVerticalGravityMiddle));
    edit_.setOnInteractiveChange(edit_fn);
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
    button_forget_.setPadding(roo_windows::PADDING_LARGE, roo_windows::PADDING_SMALL);
    button_connect_.setPadding(roo_windows::PADDING_LARGE, roo_windows::PADDING_SMALL);
    roo_display::Color pri = env.theme().color.primary;
    button_forget_.setColor(pri);
    button_connect_.setColor(pri);
    actions_.add(button_forget_,
                 roo_windows::HorizontalLayout::Params().setWeight(1));
    actions_.add(button_connect_,
                 roo_windows::HorizontalLayout::Params().setWeight(1));

    add(actions_, VerticalLayout::Params());
  }

  roo_windows::PreferredSize getPreferredSize() const override {
    return roo_windows::PreferredSize(
        roo_windows::PreferredSize::MatchParentWidth(),
        roo_windows::PreferredSize::WrapContentHeight());
  }

  void enter(const std::string& ssid) { ssid_.setText(ssid); }

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
    status_.setText(StatusAsString(status, connecting));
    if (status == WL_CONNECTED) {
      button_connect_.setCaption(kStrDisconnect);
      button_connect_.setIcon(SCALED_ROO_ICON(filled, content_clear));
      button_connect_.setColor(theme().color.primary);
      button_connect_.setOnInteractiveChange([this]() { disconnect(); });
    } else if (connecting) {
      button_connect_.setCaption(kStrConnectingEllipsis);
      button_connect_.setIcon(SCALED_ROO_ICON(filled, content_clear));
      roo_display::Color disabled = theme().color.onSurface;
      disabled.set_a(0x20);
      button_connect_.setColor(disabled);
      button_connect_.setOnInteractiveChange(nullptr);
    } else {
      button_connect_.setCaption(kStrConnect);
      button_connect_.setIcon(SCALED_ROO_ICON(filled, notification_wifi));
      button_connect_.setColor(theme().color.primary);
      button_connect_.setOnInteractiveChange([this]() { connect(); });
    }
  }

 private:
  void connect() { wifi_model_.connect(); }

  void disconnect() { wifi_model_.disconnect(); }

  Controller& wifi_model_;
  roo_toolkit::menu::Title title_;
  roo_windows::WifiIndicatorLarge indicator_;
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
