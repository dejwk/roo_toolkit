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
        start_scan_(&scheduler, [this]() { startScan(); }) {
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

  void toggleEnabled() {
    bool enabled = !wifi_.isEnabled();
    wifi_.setEnabled(enabled);
    model_listener_.onEnableChanged(enabled);
    if (enabled) {
      determineCurrentNetwork();
      resume();
    } else {
      pause();
    }
  }

  void pause() { start_scan_.cancel(); }

  void resume() {
    if (!wifi_.isEnabled()) return;
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

  void connect(const std::string& ssid, const std::string& passwd) {
    wifi_.connect(ssid, passwd);
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
    determineCurrentNetwork();
    model_listener_.onConnectionStateChanged(type);
  }

  void determineCurrentNetwork() {
    // If we're connected to the network, this is it.
    NetworkDetails current;
    if (wifi_.getApInfo(&current)) {
      current_network_.ssid = std::string((const char*)current.ssid,
                                          strlen((const char*)current.ssid));
      current_network_.open = (current.authmode == WIFI_AUTH_OPEN);
      current_network_.rssi = current.rssi;
      current_network_status_ = current.status;
    } else {
      // Check if we have a default network.
      current_network_.ssid = wifi_.store().getDefaultSSID();
      current_network_.rssi = -128;
      current_network_status_ = WL_NO_SSID_AVAIL;
      if (!current_network_.ssid.empty()) {
        // See if the default network is in range according to the latest
        // scan results.
        for (int i = 0; i < all_networks_.size(); ++i) {
          const Network& net = all_networks_[i];
          if (net.ssid == current_network_.ssid) {
            current_network_index_ = i;
            current_network_.rssi = net.rssi;
            current_network_.open = net.open;
            current_network_status_ = WL_DISCONNECTED;
            break;
          }
        }
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

  roo_scheduler::SingletonTask start_scan_;
};

}  // namespace wifi
}  // namespace roo_toolkit
