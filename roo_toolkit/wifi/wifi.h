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
  void onScanCompleted() { list_.onScanCompleted(); }

  void onCurrentNetworkChanged() { list_.onCurrentNetworkChanged(); }

  void onConnectionStateChanged(Interface::EventType type) {
    list_.onConnectionStateChanged(type);
  }

  void networkSelected(roo_windows::Task& task, const std::string& ssid) {
    if (ssid == model_.currentNetwork().ssid) {
      details_.enter(task, ssid);
      return;
    }
    const WifiModel::Network* network = model_.lookupNetwork(ssid);
    bool need_password = true;
    if (network == nullptr || network->open) {
      need_password = false;
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
