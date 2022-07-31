#pragma once

#include <unordered_set>

#include "Preferences.h"
#include "WiFi.h"
#include "roo_scheduler.h"
#include "roo_toolkit/wifi/interface.h"

namespace roo_toolkit {
namespace wifi {

class ArduinoStore : public Store {
 public:
  ArduinoStore() : preferences_() {}

  void begin() { preferences_.begin("roo/t/wifi", false); }

  ~ArduinoStore() { preferences_.end(); }

  bool getIsInterfaceEnabled() override;

  void setIsInterfaceEnabled(bool enabled) override;

  std::string getDefaultSSID() override;

  void setDefaultSSID(const std::string& ssid) override;

  bool getPassword(const std::string& ssid, std::string& password) override;

  void setPassword(const std::string& ssid,
                   const std::string& password) override;

  void clearPassword(const std::string& ssid) override;

 private:
  Preferences preferences_;
};

class ArduinoInterface : public Interface {
 public:
  ArduinoInterface(roo_scheduler::Scheduler& scheduler);

  void begin();

  bool getApInfo(NetworkDetails* info) const override;

  bool startScan() override;

  bool scanCompleted() const override;

  bool getScanResults(std::vector<NetworkDetails>* list,
                      int max_count) const override;

  void disconnect() override;

  bool connect(const std::string& ssid, const std::string& passwd) override;

  ConnectionStatus getStatus() override;

  void addEventListener(EventListener* listener) override;

  void removeEventListener(EventListener* listener) override;

 private:
  void checkStatusChanged();

  roo_scheduler::Scheduler& scheduler_;

  std::unordered_set<EventListener*> listeners_;

  ConnectionStatus status_;
  bool scanning_;
  roo_scheduler::SingletonTask check_status_changed_;
};

class ArduinoWifi {
 public:
  ArduinoWifi(roo_scheduler::Scheduler& scheduler)
      : store_(), interface_(scheduler), controller_(store_, interface_) {}

  void begin() {
    store_.begin();
    interface_.begin();
    controller_.begin();
  }

  ConnectionStatus getStatus() const { return controller_.getStatus(); }

  void setEnabled(bool enabled) { controller_.setEnabled(enabled); }

  bool isEnabled() const { return controller_.isEnabled(); }

  bool getApInfo(NetworkDetails* info) const {
    return controller_.getApInfo(info);
  }

  void addEventListener(Interface::EventListener* listener) {
    controller_.addEventListener(listener);
  }

  void removeEventListener(Interface::EventListener* listener) {
    controller_.removeEventListener(listener);
  }

  bool startScan() { return controller_.startScan(); }

  bool scanCompleted() const { return controller_.scanCompleted(); }

  bool getScanResults(std::vector<NetworkDetails>* list, int max_count) const {
    return controller_.getScanResults(list, max_count);
  }

  bool connect(const std::string& ssid, const std::string& passwd) {
    return interface_.connect(ssid, passwd);
  }

  Store& store() { return store_; }

 private:
  ArduinoStore store_;
  ArduinoInterface interface_;
  Controller controller_;
};

}  // namespace wifi
}  // namespace roo_toolkit
