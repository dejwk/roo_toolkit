#include "arduino_preferences_store.h"

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

}  // namespace

bool ArduinoPreferencesStore::getIsInterfaceEnabled() {
  return preferences_.getBool("enabled", false);
}

void ArduinoPreferencesStore::setIsInterfaceEnabled(bool enabled) {
  preferences_.putBool("enabled", enabled);
}

std::string ArduinoPreferencesStore::getDefaultSSID() {
  if (!preferences_.isKey("ssid")) return "";
  char result[33];
  preferences_.getString("ssid", result, 33);
  return std::string(result);
}

void ArduinoPreferencesStore::setDefaultSSID(const std::string& ssid) {
  preferences_.putString("ssid", ssid.c_str());
}

bool ArduinoPreferencesStore::getPassword(const std::string& ssid, std::string& password) {
  char pwkey[16];
  ToSsiPwdKey(ssid, pwkey);
  if (!preferences_.isKey(pwkey)) return false;
  char pwd[128];
  size_t len = preferences_.getString(pwkey, pwd, 128);
  password = std::string(pwd, len);
  return true;
}

void ArduinoPreferencesStore::setPassword(const std::string& ssid,
                               const std::string& password) {
  char pwkey[16];
  ToSsiPwdKey(ssid, pwkey);
  preferences_.putString(pwkey, password.c_str());
}

void ArduinoPreferencesStore::clearPassword(const std::string& ssid) {
  char pwkey[16];
  ToSsiPwdKey(ssid, pwkey);
  preferences_.remove(pwkey);
}

}  // namespace wifi
}  // namespace roo_toolkit
