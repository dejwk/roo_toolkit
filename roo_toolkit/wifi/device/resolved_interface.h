#pragma once

#include "roo_scheduler.h"
#include "roo_toolkit/wifi/hal/controller.h"
#include "roo_toolkit/wifi/hal/interface.h"

// Must be included after the <Arduino.h> header.

#ifdef ESP32

#include "roo_toolkit/wifi/device/esp32_arduino_interface.h"
#include "roo_toolkit/wifi/device/arduino_preferences_store.h"

namespace roo_toolkit {
namespace wifi {

class Esp32Wifi : public Controller {
 public:
  Esp32Wifi(roo_scheduler::Scheduler& scheduler)
      : Controller(store_, interface_, scheduler),
        store_(),
        interface_() {}

  void begin() {
    store_.begin();
    interface_.begin();
    Controller::begin();
  }

 private:
  ArduinoPreferencesStore store_;
  Esp32ArduinoInterface interface_;
};

}  // namespace wifi
}  // namespace roo_toolkit

#endif
