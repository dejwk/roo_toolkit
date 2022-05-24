#include "WiFi.h"
#include "roo_toolkit/wifi/interface.h"
#include "roo_toolkit/wifi/network_details.h"

namespace roo_toolkit {
namespace wifi {

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

class ArduinoInterface : public Interface {
 public:
  ArduinoInterface() { WiFi.mode(WIFI_STA); }

  bool getApInfo(NetworkDetails* info) const override {
    if (!WiFi.isConnected()) return false;
    auto ssid = WiFi.SSID();
    memcpy(info->ssid, ssid.c_str(), ssid.length());
    info->ssid[ssid.length() + 1] = 0;
    info->authmode = WIFI_AUTH_UNKNOWN; //authMode(WiFi.encryptionType());
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
  }

  bool startScan() override {
    return WiFi.scanNetworks(true, false) == WIFI_SCAN_RUNNING;
  }

  bool scanCompleted() const override { return WiFi.scanComplete() >= 0; }

  bool getScanResults(std::vector<NetworkDetails>* list,
                      int max_count) const override {
    int16_t result = WiFi.scanComplete();
    if (result < 0) return false;
    if (max_count > result) {
      max_count = result;
    }
    list->clear();
    for (int i = 0; i < max_count; ++i) {
      NetworkDetails info;
      // String ssid;
      // uint8_t encryptionType;
      // int32_t rssi;
      // WiFi.getNetworkInfo(i, ssid, encryptionType, &RSSI, uint8_t* &BSSID, int32_t &channel);

      auto ssid = WiFi.SSID(i);
      memcpy(info.ssid, ssid.c_str(), ssid.length());
      info.ssid[ssid.length()] = 0;
      info.authmode = authMode(WiFi.encryptionType(i));
      info.rssi = WiFi.RSSI(i);
      // auto mac = WiFi.macAddress(i);
      // memcpy(info->bssid, mac.c_str(), 6);
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
};

}  // namespace wifi
}  // namespace roo_toolkit
