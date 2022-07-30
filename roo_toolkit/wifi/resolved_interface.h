#pragma once

#include "roo_scheduler.h"
#include "roo_toolkit/wifi/interface.h"

// Must be included after the <Arduino.h> header.

#ifdef ARDUINO

#include "roo_toolkit/wifi/arduino_interface.h"

namespace roo_toolkit {
namespace wifi {

typedef ArduinoWifi Wifi;

}  // namespace wifi
}  // namespace roo_toolkit

#endif
