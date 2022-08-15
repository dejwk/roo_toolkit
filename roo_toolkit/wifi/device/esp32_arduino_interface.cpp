#include "esp32_arduino_interface.h"

namespace roo_toolkit {
namespace wifi {

namespace {

static AuthMode authMode(wifi_auth_mode_t mode) {
  switch (mode) {
    case ::WIFI_AUTH_OPEN:
      return WIFI_AUTH_OPEN;
    case ::WIFI_AUTH_WEP:
      return WIFI_AUTH_WEP;
    case ::WIFI_AUTH_WPA_PSK:
      return WIFI_AUTH_WPA_PSK;
    case ::WIFI_AUTH_WPA2_PSK:
      return WIFI_AUTH_WPA2_PSK;
    case ::WIFI_AUTH_WPA_WPA2_PSK:
      return WIFI_AUTH_WPA_WPA2_PSK;
    case ::WIFI_AUTH_WPA2_ENTERPRISE:
      return WIFI_AUTH_WPA2_ENTERPRISE;
    default:
      return WIFI_AUTH_UNKNOWN;
  }
}

}  // namespace

Esp32ArduinoInterface::Esp32ArduinoInterface(
    roo_scheduler::Scheduler& scheduler)
    : scheduler_(scheduler),
      status_(WL_DISCONNECTED),
      scanning_(false),
      check_status_changed_(&scheduler, [this]() { checkStatusChanged(); }) {}

void Esp32ArduinoInterface::begin() {
  checkStatusChanged();
  WiFi.mode(WIFI_STA);
  // // #ifdef ESP32
  // WiFi.onEvent(
  //     [this](system_event_id_t event) {
  //       for (const auto& l : listeners_) {
  //         l->scanCompleted();
  //       }
  //     },
  //     SYSTEM_EVENT_SCAN_DONE);
  // // #endif
}

bool Esp32ArduinoInterface::getApInfo(NetworkDetails* info) const {
  const String& ssid = WiFi.SSID();
  if (ssid.length() == 0) return false;
  memcpy(info->ssid, ssid.c_str(), ssid.length());
  info->ssid[ssid.length()] = 0;
  info->authmode = WIFI_AUTH_UNKNOWN;  // authMode(WiFi.encryptionType());
  info->rssi = WiFi.RSSI();
  auto mac = WiFi.macAddress();
  memcpy(info->bssid, mac.c_str(), 6);
  info->primary = WiFi.channel();
  info->group_cipher = WIFI_CIPHER_TYPE_UNKNOWN;
  info->pairwise_cipher = WIFI_CIPHER_TYPE_UNKNOWN;
  info->use_11b = false;
  info->use_11g = false;
  info->use_11n = false;
  info->supports_wps = false;

  info->status = (ConnectionStatus)WiFi.status();
  return true;
}

bool Esp32ArduinoInterface::startScan() {
  scanning_ = (WiFi.scanNetworks(true, false) == WIFI_SCAN_RUNNING);
  return scanning_;
}

bool Esp32ArduinoInterface::scanCompleted() const {
  bool completed = WiFi.scanComplete() >= 0;
  return completed;
}

bool Esp32ArduinoInterface::getScanResults(std::vector<NetworkDetails>* list,
                                           int max_count) const {
  int16_t result = WiFi.scanComplete();
  if (result < 0) return false;
  if (max_count > result) {
    max_count = result;
  }
  list->clear();
  for (int i = 0; i < max_count; ++i) {
    NetworkDetails info;
    auto ssid = WiFi.SSID(i);
    memcpy(info.ssid, ssid.c_str(), ssid.length());
    info.ssid[ssid.length()] = 0;
    info.authmode = authMode(WiFi.encryptionType(i));
    info.rssi = WiFi.RSSI(i);
    info.primary = WiFi.channel(i);
    info.group_cipher = WIFI_CIPHER_TYPE_UNKNOWN;
    info.pairwise_cipher = WIFI_CIPHER_TYPE_UNKNOWN;
    info.use_11b = false;
    info.use_11g = false;
    info.use_11n = false;
    info.supports_wps = false;
    list->push_back(std::move(info));
  }
  return true;
}

void Esp32ArduinoInterface::disconnect() { WiFi.disconnect(); }

bool Esp32ArduinoInterface::connect(const std::string& ssid,
                                    const std::string& passwd) {
  WiFi.begin(ssid.c_str(), passwd.c_str());
  return true;
}

ConnectionStatus Esp32ArduinoInterface::getStatus() {
  return (ConnectionStatus)WiFi.status();
}

void Esp32ArduinoInterface::addEventListener(EventListener* listener) {
  listeners_.insert(listener);
}

void Esp32ArduinoInterface::removeEventListener(EventListener* listener) {
  listeners_.erase(listener);
}

namespace {

Interface::EventType getEventType(ConnectionStatus status) {
  switch (status) {
    case WL_IDLE_STATUS:
      return Interface::EV_CONNECTED;
    case WL_CONNECTED:
      return Interface::EV_GOT_IP;
    case WL_DISCONNECTED:
      return Interface::EV_DISCONNECTED;
    case WL_CONNECT_FAILED:
      return Interface::EV_CONNECTION_FAILED;
    case WL_CONNECTION_LOST:
      return Interface::EV_CONNECTION_LOST;
    default:
      return Interface::EV_UNKNOWN;
  }
}

}  // namespace

void Esp32ArduinoInterface::checkStatusChanged() {
  ConnectionStatus new_status = getStatus();
  if (new_status != status_) {
    status_ = new_status;
    EventType type = getEventType(status_);
    for (const auto& l : listeners_) {
      l->onEvent(type);
    }
  }
  if (scanning_ && WiFi.scanComplete() >= 0) {
    scanning_ = false;
    for (const auto& l : listeners_) {
      l->onEvent(EV_SCAN_COMPLETED);
    }
  }

  check_status_changed_.scheduleAfter(roo_time::Millis(500));
}

}  // namespace wifi
}  // namespace roo_toolkit
