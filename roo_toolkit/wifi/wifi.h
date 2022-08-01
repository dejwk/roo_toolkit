#pragma once

#include <Arduino.h>

#include "roo_toolkit/wifi/enter_password_activity.h"
#include "roo_toolkit/wifi/list_activity.h"
#include "roo_toolkit/wifi/model.h"
#include "roo_toolkit/wifi/network_details_activity.h"
#include "roo_toolkit/wifi/resolved_interface.h"

namespace roo_toolkit {
namespace wifi {

class WifiSetup {
 public:
  WifiSetup(const roo_windows::Environment& env, Wifi& wifi,
            roo_windows::TextFieldEditor& editor,
            roo_scheduler::Scheduler& scheduler)
      : model_listener_(*this),
        model_(wifi, scheduler, model_listener_),
        list_(env, model_,
              [this](roo_windows::Task& task, const std::string& ssid) {
                networkSelected(task, ssid);
              }),
        details_(env, model_),
        enter_password_(env, editor, model_) {}

  roo_windows::Activity& main() { return list_; }

  roo_windows::Activity& enter_password() { return enter_password_; }

 private:
  class ModelListener : public WifiModel::Listener {
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
    const WifiModel::Network* network = model_.lookupNetwork(ssid);
    std::string password;
    bool same_network = (ssid == model_.currentNetwork().ssid);
    bool has_password = false;
    if (!same_network || model_.currentNetworkStatus() != WL_CONNECT_FAILED) {
      has_password = model_.getStoredPassword(ssid, password);
    }
    bool need_password =
        (network != nullptr && !network->open && !has_password);
    if (!need_password &&
        (!same_network || (model_.currentNetworkStatus() == WL_DISCONNECTED &&
                           !model_.isConnecting()))) {
      // Clicked on an open or remembered network to which we are not already
      // connected or connecting. Interpret as a pure 'action' intent.
      model_.connect(ssid, password);
      return;
    }
    if (need_password) {
      task.enterActivity(&enter_password_);
      enter_password_.enter(ssid);
    } else {
      details_.enter(task, ssid);
    }
  }

  ModelListener model_listener_;
  WifiModel model_;
  ListActivity list_;
  NetworkDetailsActivity details_;
  EnterPasswordActivity enter_password_;
};

}  // namespace wifi
}  // namespace roo_toolkit
