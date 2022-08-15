#pragma once

#include <Arduino.h>

#include "roo_toolkit/wifi/activity/enter_password_activity.h"
#include "roo_toolkit/wifi/activity/list_activity.h"
#include "roo_toolkit/wifi/activity/network_details_activity.h"
#include "roo_toolkit/wifi/device/resolved_interface.h"

namespace roo_toolkit {
namespace wifi {

class WifiSetup {
 public:
  WifiSetup(const roo_windows::Environment& env, Controller& controller,
            roo_windows::TextFieldEditor& editor,
            roo_scheduler::Scheduler& scheduler)
      : controller_(controller),
        model_listener_(*this),
        // model_(controller, scheduler, model_listener_),
        list_(env, controller_,
              [this](roo_windows::Task& task, const std::string& ssid) {
                networkSelected(task, ssid);
              }),
        details_(env, controller_,
                 [this](roo_windows::Task& task, const std::string& ssid) {
                   networkEdited(task, ssid);
                 }),
        enter_password_(env, editor, controller_) {
    controller_.addListener(&model_listener_);
  }

  roo_windows::Activity& main() { return list_; }

  roo_windows::Activity& enter_password() { return enter_password_; }

  ~WifiSetup() { controller_.removeListener(&model_listener_); }

 private:
  class ModelListener : public Controller::Listener {
   public:
    ModelListener(WifiSetup& wifi) : wifi_(wifi) {}

    void onEnableChanged(bool enabled) override {
      wifi_.onEnableChanged(enabled);
    }

    void onScanStarted() override { wifi_.onScanStarted(); }
    void onScanCompleted() override { wifi_.onScanCompleted(); }

    void onCurrentNetworkChanged() override { wifi_.onCurrentNetworkChanged(); }

    void onConnectionStateChanged(Interface::EventType type) override {
      wifi_.onConnectionStateChanged(type);
    }

   private:
    WifiSetup& wifi_;
  };

  friend class ModelListener;

  void onEnableChanged(bool enabled) { list_.onEnableChanged(enabled); }

  void onScanStarted() { list_.onScanStarted(); }

  void onScanCompleted() {
    list_.onScanCompleted();
    details_.onScanCompleted();
  }

  void onCurrentNetworkChanged() {
    list_.onCurrentNetworkChanged();
    details_.onCurrentNetworkChanged();
  }

  void onConnectionStateChanged(Interface::EventType type) {
    list_.onConnectionStateChanged(type);
    details_.onCurrentNetworkChanged();
  }

  void networkSelected(roo_windows::Task& task, const std::string& ssid) {
    const Controller::Network* network = controller_.lookupNetwork(ssid);
    std::string password;
    bool same_network = (ssid == controller_.currentNetwork().ssid);
    bool has_password = false;
    if (!same_network ||
        controller_.currentNetworkStatus() != WL_CONNECT_FAILED) {
      has_password = controller_.getStoredPassword(ssid, password);
    }
    bool need_password =
        (network != nullptr && !network->open && !has_password);
    if (!need_password &&
        (!same_network ||
         (controller_.currentNetworkStatus() == WL_DISCONNECTED &&
          !controller_.isConnecting()))) {
      // Clicked on an open or remembered network to which we are not already
      // connected or connecting. Interpret as a pure 'action' intent.
      controller_.connect(ssid, password);
      return;
    }
    if (need_password) {
      enter_password_.enter(task, ssid, "enter password");
    } else {
      details_.enter(task, ssid);
    }
  }

  void networkEdited(roo_windows::Task& task, const std::string& ssid) {
    enter_password_.enter(task, ssid, "(unchanged)");
  }

  Controller& controller_;
  ModelListener model_listener_;
  ListActivity list_;
  NetworkDetailsActivity details_;
  EnterPasswordActivity enter_password_;
};

}  // namespace wifi
}  // namespace roo_toolkit
