#pragma once

#include <unordered_set>

#include "Preferences.h"
#include "WiFi.h"
#include "roo_scheduler.h"
#include "roo_toolkit/wifi/hal/controller.h"
#include "roo_toolkit/wifi/hal/interface.h"
#include "roo_toolkit/wifi/hal/store.h"
#include "roo_toolkit/wifi/device/arduino_preferences_store.h"

namespace roo_toolkit {
namespace wifi {

class Esp32ArduinoInterface : public Interface {
 public:
  Esp32ArduinoInterface(roo_scheduler::Scheduler& scheduler);

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

}  // namespace wifi
}  // namespace roo_toolkit
