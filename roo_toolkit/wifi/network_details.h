#pragma once

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
};

}  // namespace wifi
}  // namespace roo_toolkit
