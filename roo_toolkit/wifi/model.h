#pragma once

#include <Arduino.h>

#include "roo_toolkit/wifi/resolved_interface.h"

namespace roo_toolkit {
namespace wifi {

class WifiModel {
 public:
  struct Network {
    Network() : ssid(), open(false), rssi(-128) {}

    std::string ssid;
    bool open;
    int8_t rssi;
  };

  class Listener {
   public:
    virtual ~Listener() {}

    virtual void onEnableChanged(bool enabled) {}
    virtual void onScanStarted() {}
    virtual void onScanCompleted() {}
    virtual void onCurrentNetworkChanged() {}
    virtual void onConnectionStateChanged(Interface::EventType type) {}
  };

  WifiModel(Wifi& wifi, roo_scheduler::Scheduler& scheduler, Listener& listener)
      : wifi_(wifi),
        current_network_(),
        current_network_index_(-1),
        current_network_status_(WL_NO_SSID_AVAIL),
        all_networks_(),
        wifi_listener_(*this),
        model_listener_(listener),
        connecting_(false),
        start_scan_(&scheduler, [this]() { startScan(); }),
        refresh_current_network_(
            &scheduler, [this]() { periodicRefreshCurrentNetwork(); }) {
    wifi_.addEventListener(&wifi_listener_);
  }

  ~WifiModel() { wifi_.removeEventListener(&wifi_listener_); }

  int otherScannedNetworksCount() const {
    int count = all_networks_.size();
    if (current_network_index_ >= 0) --count;
    return count;
  }

  const Network& currentNetwork() const { return current_network_; }

  const Network* lookupNetwork(const std::string& ssid) const {
    for (const Network& net : all_networks_) {
      if (net.ssid == ssid) return &net;
    }
    return nullptr;
  }

  ConnectionStatus currentNetworkStatus() const {
    return current_network_status_;
  }

  const Network& otherNetwork(int idx) const {
    if (current_network_index_ >= 0 && idx >= current_network_index_) {
      idx++;
    }
    return all_networks_[idx];
  }

  bool startScan() {
    bool started = wifi_.startScan();
    if (started) {
      model_listener_.onScanStarted();
    }
    return started;
  }

  bool isScanCompleted() const { return wifi_.scanCompleted(); }
  bool isEnabled() const { return wifi_.isEnabled(); }

  bool isConnecting() const { return connecting_; }

  void toggleEnabled() {
    bool enabled = !wifi_.isEnabled();
    wifi_.setEnabled(enabled);
    model_listener_.onEnableChanged(enabled);
    if (enabled) {
      resume();
    } else {
      pause();
    }
  }

  bool getStoredPassword(const std::string& ssid, std::string passwd) const {
    return wifi_.store().getPassword(ssid, passwd);
  }

  void pause() { start_scan_.cancel(); }

  void resume() {
    if (!wifi_.isEnabled()) return;
    refreshCurrentNetwork();
    if (!refresh_current_network_.isScheduled()) {
      refresh_current_network_.scheduleAfter(roo_time::Seconds(2));
    }
    if (wifi_.scanCompleted()) {
      model_listener_.onScanCompleted();
      start_scan_.scheduleAfter(roo_time::Seconds(15));
    } else {
      startScan();
    }
  }

  void setPassword(const std::string& ssid, const std::string& passwd) {
    wifi_.store().setPassword(ssid, passwd);
  }

  bool connect(const std::string& ssid, const std::string& passwd) {
    if (!wifi_.connect(ssid, passwd)) return false;
    connecting_ = true;
    std::string default_ssid = wifi_.store().getDefaultSSID();
    if (ssid != default_ssid) {
      wifi_.store().setDefaultSSID(ssid);
    }
    std::string current_password;
    if (!passwd.empty() &&
        (!wifi_.store().getPassword(ssid, current_password) ||
         current_password != passwd)) {
      wifi_.store().setPassword(ssid, passwd);
    }
    refreshCurrentNetwork();
  }

 private:
  class WifiListener : public Interface::EventListener {
   public:
    WifiListener(WifiModel& wifi) : wifi_(wifi) {}

    void onEvent(Interface::EventType type) {
      switch (type) {
        case Interface::EV_SCAN_COMPLETED: {
          wifi_.onScanCompleted();
          break;
        }
        default: {
          wifi_.onConnectionStateChanged(type);
          break;
        }
      }
    }

   private:
    WifiModel& wifi_;
  };

  friend class WifiListener;

  void onConnectionStateChanged(Interface::EventType type) {
    connecting_ = false;
    refreshCurrentNetwork();
    model_listener_.onConnectionStateChanged(type);
  }

  void periodicRefreshCurrentNetwork() {
    refreshCurrentNetwork();
    if (isEnabled()) {
      refresh_current_network_.scheduleAfter(roo_time::Seconds(2));
    }
  }

  void refreshCurrentNetwork() {
    // If we're connected to the network, this is it.
    NetworkDetails current;
    if (wifi_.getApInfo(&current)) {
      updateCurrentNetwork(std::string((const char*)current.ssid,
                                       strlen((const char*)current.ssid)),
                           (current.authmode == WIFI_AUTH_OPEN), current.rssi,
                           current.status);
    } else {
      // Check if we have a default network.
      std::string default_ssid = wifi_.store().getDefaultSSID();
      const Network* default_network_in_range = nullptr;
      if (!default_ssid.empty()) {
        // See if the default network is in range according to the latest
        // scan results.
        default_network_in_range = lookupNetwork(default_ssid);
      }
      if (default_network_in_range == nullptr) {
        updateCurrentNetwork(default_ssid, true, -128, WL_NO_SSID_AVAIL);
      } else {
        updateCurrentNetwork(default_ssid, default_network_in_range->open,
                             default_network_in_range->rssi, WL_DISCONNECTED);
      }
    }
  }

  void updateCurrentNetwork(const std::string& ssid, bool open, int8_t rssi,
                            ConnectionStatus status) {
    if (rssi == current_network_.rssi && ssid == current_network_.ssid &&
        open == current_network_.open && status == current_network_status_) {
      return;
    }
    current_network_.ssid = ssid;
    current_network_.open = open;
    current_network_.rssi = rssi;
    current_network_status_ = status;
    current_network_index_ = -1;
    for (int i = 0; i < all_networks_.size(); ++i) {
      if (all_networks_[i].ssid == ssid) {
        current_network_index_ = i;
        break;
      }
    }
    model_listener_.onCurrentNetworkChanged();
  }

  void onScanCompleted() {
    current_network_status_ = WL_NO_SSID_AVAIL;
    current_network_index_ = -1;
    std::vector<NetworkDetails> raw_data;
    wifi_.getScanResults(&raw_data, 100);
    int raw_count = raw_data.size();
    if (raw_count == 0) {
      all_networks_.clear();
      return;
    }
    // De-duplicate SSID, keeping the one with the strongest signal.
    // Start by sorting by (ssid, signal strength).
    std::vector<uint8_t> indices(raw_data.size(), 0);
    for (uint8_t i = 0; i < raw_count; ++i) indices[i] = i;
    std::sort(&indices[0], &indices[raw_count], [&](int a, int b) -> bool {
      int ssid_cmp = strncmp((const char*)raw_data[a].ssid,
                             (const char*)raw_data[b].ssid, 33);
      if (ssid_cmp < 0) return true;
      if (ssid_cmp > 0) return false;
      return raw_data[a].rssi > raw_data[b].rssi;
    });
    // Now, compact the result by keeping the first value for each SSID.
    const char* current_ssid = (const char*)raw_data[indices[0]].ssid;
    uint8_t src = 1;
    uint8_t dst = 1;
    while (src < raw_count) {
      const char* candidate_ssid = (const char*)raw_data[indices[src]].ssid;
      if (strncmp(current_ssid, candidate_ssid, 33) != 0) {
        current_ssid = candidate_ssid;
        indices[dst++] = indices[src];
      }
      ++src;
    }
    // Now sort again, this time by signal strength only.
    // Single-out and remove the default network.
    std::sort(&indices[0], &indices[dst], [&](int a, int b) -> bool {
      return raw_data[a].rssi > raw_data[b].rssi;
    });
    // Finally, copy over the results.
    all_networks_.resize(dst);
    for (uint8_t i = 0; i < dst; ++i) {
      NetworkDetails& src = raw_data[indices[i]];
      Network& dst = all_networks_[i];
      dst.ssid =
          std::string((const char*)src.ssid, strlen((const char*)src.ssid));
      dst.open = (src.authmode == WIFI_AUTH_OPEN);
      dst.rssi = src.rssi;
      if (dst.ssid == current_network_.ssid) {
        current_network_index_ = i;
      }
    }
    model_listener_.onScanCompleted();
    if (wifi_.isEnabled()) {
      start_scan_.scheduleAfter(roo_time::Seconds(15));
    }
  }

  Wifi& wifi_;
  Network current_network_;
  int16_t current_network_index_;
  ConnectionStatus current_network_status_;
  std::vector<Network> all_networks_;
  WifiListener wifi_listener_;
  Listener& model_listener_;
  bool connecting_;

  roo_scheduler::SingletonTask start_scan_;
  roo_scheduler::SingletonTask refresh_current_network_;
};

inline constexpr const char* StatusAsString(ConnectionStatus status,
                                            bool connecting) {
  return (status == WL_IDLE_STATUS)       ? "Connected, no Internet"
         : (status == WL_NO_SSID_AVAIL)   ? "Out of range"
         : (status == WL_CONNECTED)       ? "Connected"
         : (status == WL_CONNECT_FAILED)  ? "Check password and try again"
         : (status == WL_CONNECTION_LOST) ? "Connection lost"
         : (status == WL_DISCONNECTED)
             ? (connecting ? "Connecting" : "Disconnected")
             : "Unknown";
}

}  // namespace wifi
}  // namespace roo_toolkit
