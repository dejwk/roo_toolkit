#include "arduino_interface.h"

namespace roo_toolkit {
namespace wifi {

namespace {

uint64_t inline MurmurOAAT64(const char* key) {
  uint64_t h(525201411107845655ull);
  for (; *key != '\0'; ++key) {
    h ^= *key;
    h *= 0x5bd1e9955bd1e995;
    h ^= h >> 47;
  }
  return h;
}

void ToSsiPwdKey(const std::string& ssid, char* result) {
  uint64_t hash = MurmurOAAT64(ssid.c_str());
  *result++ = 'p';
  *result++ = 'w';
  *result++ = '-';
  // We break 64 bits into 11 groups of 6 bits; then to ASCII.
  for (int i = 0; i < 11; i++) {
    *result++ = (hash & 0x3F) + 48;  // ASCII from '0' to 'o'.
    hash >>= 6;
  }
  *result = '\0';
}

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

bool ArduinoStore::getIsInterfaceEnabled() {
  return preferences_.getBool("enabled", false);
}

void ArduinoStore::setIsInterfaceEnabled(bool enabled) {
  preferences_.putBool("enabled", enabled);
}

std::string ArduinoStore::getDefaultSSID() {
  if (!preferences_.isKey("ssid")) return "";
  char result[33];
  preferences_.getString("ssid", result, 33);
  return std::string(result);
}

void ArduinoStore::setDefaultSSID(const std::string& ssid) {
  preferences_.putString("ssid", ssid.c_str());
}

bool ArduinoStore::getPassword(const std::string& ssid, std::string& password) {
  char pwkey[16];
  ToSsiPwdKey(ssid, pwkey);
  if (!preferences_.isKey(pwkey)) return false;
  char pwd[128];
  preferences_.getString(pwkey, pwd, 128);
  return true;
}

void ArduinoStore::setPassword(const std::string& ssid,
                               const std::string& password) {
  char pwkey[16];
  ToSsiPwdKey(ssid, pwkey);
  preferences_.putString(pwkey, password.c_str());
}

void ArduinoStore::clearPassword(const std::string& ssid) {
  char pwkey[16];
  ToSsiPwdKey(ssid, pwkey);
  preferences_.remove(pwkey);
}

ArduinoInterface::ArduinoInterface(roo_scheduler::Scheduler& scheduler)
    : scheduler_(scheduler),
      status_(WL_DISCONNECTED),
      scanning_(false),
      check_status_changed_(&scheduler, [this]() { checkStatusChanged(); }) {}

void ArduinoInterface::begin() {
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

bool ArduinoInterface::getApInfo(NetworkDetails* info) const {
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

bool ArduinoInterface::startScan() {
  return (WiFi.scanNetworks(true, false) == WIFI_SCAN_RUNNING);
}

bool ArduinoInterface::scanCompleted() const {
  bool completed = WiFi.scanComplete() >= 0;
  return completed;
}

bool ArduinoInterface::getScanResults(std::vector<NetworkDetails>* list,
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

void ArduinoInterface::disconnect() { WiFi.disconnect(); }

bool ArduinoInterface::connect(const std::string& ssid,
                               const std::string& passwd) {
  return WiFi.begin(ssid.c_str(), passwd.c_str()) != WL_CONNECT_FAILED;
}

ConnectionStatus ArduinoInterface::getStatus() {
  return (ConnectionStatus)WiFi.status();
}

void ArduinoInterface::addEventListener(EventListener* listener) {
  listeners_.insert(listener);
}

void ArduinoInterface::removeEventListener(EventListener* listener) {
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

void ArduinoInterface::checkStatusChanged() {
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
