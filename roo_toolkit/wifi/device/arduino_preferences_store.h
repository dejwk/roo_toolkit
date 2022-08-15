#pragma once

#include <unordered_set>

#include "Preferences.h"
#include "WiFi.h"
#include "roo_scheduler.h"
#include "roo_toolkit/wifi/hal/controller.h"
#include "roo_toolkit/wifi/hal/interface.h"
#include "roo_toolkit/wifi/hal/store.h"

namespace roo_toolkit {
namespace wifi {

class ArduinoPreferencesStore : public Store {
 public:
  ArduinoPreferencesStore() : preferences_() {}

  void begin() { preferences_.begin("roo/t/wifi", false); }

  ~ArduinoPreferencesStore() { preferences_.end(); }

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

}  // namespace wifi
}  // namespace roo_toolkit
