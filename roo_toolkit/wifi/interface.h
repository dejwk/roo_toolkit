#pragma once

#include <inttypes.h>
#include <string>
#include <vector>

namespace roo_toolkit {
namespace wifi {

enum AuthMode {
  WIFI_AUTH_OPEN = 0,        /**< authenticate mode : open */
  WIFI_AUTH_WEP,             /**< authenticate mode : WEP */
  WIFI_AUTH_WPA_PSK,         /**< authenticate mode : WPA_PSK */
  WIFI_AUTH_WPA2_PSK,        /**< authenticate mode : WPA2_PSK */
  WIFI_AUTH_WPA_WPA2_PSK,    /**< authenticate mode : WPA_WPA2_PSK */
  WIFI_AUTH_WPA2_ENTERPRISE, /**< authenticate mode : WPA2_ENTERPRISE */
  WIFI_AUTH_WPA3_PSK,        /**< authenticate mode : WPA3_PSK */
  WIFI_AUTH_WPA2_WPA3_PSK,   /**< authenticate mode : WPA2_WPA3_PSK */
  WIFI_AUTH_WAPI_PSK,        /**< authenticate mode : WAPI_PSK */
  WIFI_AUTH_UNKNOWN
};

enum CipherType {
  WIFI_CIPHER_TYPE_NONE = 0,    /**< the cipher type is none */
  WIFI_CIPHER_TYPE_WEP40,       /**< the cipher type is WEP40 */
  WIFI_CIPHER_TYPE_WEP104,      /**< the cipher type is WEP104 */
  WIFI_CIPHER_TYPE_TKIP,        /**< the cipher type is TKIP */
  WIFI_CIPHER_TYPE_CCMP,        /**< the cipher type is CCMP */
  WIFI_CIPHER_TYPE_TKIP_CCMP,   /**< the cipher type is TKIP and CCMP */
  WIFI_CIPHER_TYPE_AES_CMAC128, /**< the cipher type is AES-CMAC-128 */
  WIFI_CIPHER_TYPE_SMS4,        /**< the cipher type is SMS4 */
  WIFI_CIPHER_TYPE_GCMP,        /**< the cipher type is GCMP */
  WIFI_CIPHER_TYPE_GCMP256,     /**< the cipher type is GCMP-256 */
  WIFI_CIPHER_TYPE_AES_GMAC128, /**< the cipher type is AES-GMAC-128 */
  WIFI_CIPHER_TYPE_AES_GMAC256, /**< the cipher type is AES-GMAC-256 */
  WIFI_CIPHER_TYPE_UNKNOWN,     /**< the cipher type is unknown */
};

enum ConnectionStatus {
  WL_IDLE_STATUS = 0,
  WL_NO_SSID_AVAIL = 1,
  WL_SCAN_COMPLETED = 2,
  WL_CONNECTED = 3,
  WL_CONNECT_FAILED = 4,
  WL_CONNECTION_LOST = 5,
  WL_DISCONNECTED = 6
};

struct NetworkDetails {
  uint8_t bssid[6];           /**< MAC address of AP */
  uint8_t ssid[33];           /**< SSID of AP */
  uint8_t primary;            /**< channel of AP */
  int8_t rssi;                /**< signal strength of AP */
  AuthMode authmode;          /**< authmode of AP */
  CipherType pairwise_cipher; /**< pairwise cipher of AP */
  CipherType group_cipher;    /**< group cipher of AP */
  bool use_11b;
  bool use_11g;
  bool use_11n;
  bool supports_wps;

  ConnectionStatus status;
};

// Abstraction for persistently storing the data used by the WiFi controller.
class Store {
 public:
  virtual bool getIsInterfaceEnabled() = 0;
  virtual void setIsInterfaceEnabled(bool enabled) = 0;
  virtual std::string getDefaultSSID() = 0;
  virtual void setDefaultSSID(const std::string& ssid) = 0;
  virtual bool getPassword(const std::string& ssid, std::string& password) = 0;
  virtual void setPassword(const std::string& ssid,
                           const std::string& password) = 0;
  virtual void clearPassword(const std::string& ssid) = 0;
};

// Abstraction for interacting with the hardware WiFi interface.
class Interface {
 public:
  // enum ConnectionFailure {
  //   UNSPECIFIED_FAILURE = 1,
  //   AUTH_EXPIRED = 2,

  //   BEACON_TIMEOUT = 200,
  //   NO_AP_FOUND = 201,
  //   AUTH_FAILURE = 202,
  //   ASSOCIATION_FAILURE = 203,
  //   HANDSHAKE_TIMEOUT = 204,
  //   CONNECTION_FAILURE = 205,
  //   AP_TSF_RESET = 206,
  //   ROAMING = 207,
  // };

  enum EventType {
    EV_UNKNOWN = 0,
    EV_SCAN_COMPLETED = 1,
    EV_CONNECTED = 2,
    EV_GOT_IP = 3,
    EV_DISCONNECTED = 4,
    EV_CONNECTION_FAILED = 5,
    EV_CONNECTION_LOST = 6,
  };

  class EventListener {
   public:
    virtual ~EventListener() {}
    virtual void onEvent(EventType type) {}
    // virtual void scanCompleted() {}
    // virtual void connected() {}
    // virtual void disconnected() {}
    // virtual void gotIP() {}
    // virtual void connectionFailed(ConnectionFailure failure) {}
  };

  virtual void addEventListener(EventListener* listener) = 0;
  virtual void removeEventListener(EventListener* listener) = 0;

  virtual bool getApInfo(NetworkDetails* info) const = 0;
  virtual bool startScan() = 0;
  virtual bool scanCompleted() const = 0;

  virtual void disconnect() = 0;
  virtual void connect(const std::string& ssid, const std::string& passwd) = 0;
  virtual ConnectionStatus getStatus() = 0;

  virtual bool getScanResults(std::vector<NetworkDetails>* list,
                              int max_count) const = 0;
  virtual ~Interface() {}
};

class Controller {
 public:
  Controller(Store& store, Interface& interface)
      : store_(store),
        interface_(interface),
        enabled_(false),
        connecting_(false) {}

  void begin() {
    enabled_ = store_.getIsInterfaceEnabled();
    ssid_ = store_.getDefaultSSID();
    if (enabled_ && !ssid_.empty()) {
      std::string password;
      if (store_.getPassword(ssid_, password)) {
        connecting_ = true;
        interface_.connect(ssid_, password);
      }
    }
  }

  void setEnabled(bool enabled) {
    enabled_ = enabled;
    store_.setIsInterfaceEnabled(enabled);
    if (!enabled_) {
      interface_.disconnect();
    }
  }

  bool isEnabled() const { return enabled_; }

  void addEventListener(Interface::EventListener* listener) {
    interface_.addEventListener(listener);
  }

  void removeEventListener(Interface::EventListener* listener) {
    interface_.removeEventListener(listener);
  }

  bool getApInfo(NetworkDetails* info) const {
    return interface_.getApInfo(info);
  }

  bool startScan() { return interface_.startScan(); }

  bool scanCompleted() const { return interface_.scanCompleted(); }

  bool getScanResults(std::vector<NetworkDetails>* list, int max_count) const {
    return interface_.getScanResults(list, max_count);
  }

 private:
  Store& store_;
  Interface& interface_;
  bool enabled_;
  bool connecting_;
  std::string ssid_;
};

}  // namespace wifi
}  // namespace roo_toolkit
