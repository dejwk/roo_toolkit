#pragma once

#include <inttypes.h>

#include <string>
#include <vector>

#include "roo_toolkit/wifi/activity/resources.h"

#include "roo_toolkit/wifi/hal/interface.h"
#include "roo_toolkit/wifi/hal/store.h"

namespace roo_toolkit {
namespace wifi {

namespace {

ConnectionStatus getConnectionStatus(Interface::EventType type) {
  switch (type) {
    case Interface::EV_CONNECTED:
      return WL_IDLE_STATUS;
    case Interface::EV_GOT_IP:
      return WL_CONNECTED;
    case Interface::EV_DISCONNECTED:
      return WL_DISCONNECTED;
    case Interface::EV_CONNECTION_FAILED:
      return WL_CONNECT_FAILED;
    case Interface::EV_CONNECTION_LOST:
      return WL_CONNECTION_LOST;
    default:
      return WL_CONNECT_FAILED;
  }
}

}  // namespace

class Controller {
 public:
  struct Network {
    Network() : ssid(), open(false), rssi(-128) {}

    std::string ssid;
    bool open;
    int8_t rssi;
  };

  class Listener {
   public:
    Listener() : next_(nullptr), prev_(nullptr) {}

    virtual ~Listener() {}

    virtual void onEnableChanged(bool enabled) {}
    virtual void onScanStarted() {}
    virtual void onScanCompleted() {}
    virtual void onCurrentNetworkChanged() {}
    virtual void onConnectionStateChanged(Interface::EventType type) {}

   private:
    friend class Controller;

    Listener* attach(Listener* list) {
      if (next_ != nullptr || prev_ != nullptr) return this;
      if (list == nullptr) {
        this->next_ = this->prev_ = this;
      } else {
        this->next_ = list->next_;
        this->prev_ = list;
        list->next_->prev_ = this;
        list->next_ = this;
      }
      return this;
    }

    Listener* detach() {
      Listener* result = nullptr;
      if (next_ != prev_) {
        next_->prev_ = prev_;
        prev_->next_ = next_;
        result = next_;
      }
      next_ = nullptr;
      prev_ = nullptr;
      return result;
    }

    Listener* next_;
    Listener* prev_;
  };

  Controller(Store& store, Interface& interface,
             roo_scheduler::Scheduler& scheduler)
      : store_(store),
        interface_(interface),
        enabled_(false),
        current_network_(),
        current_network_index_(-1),
        current_network_status_(WL_NO_SSID_AVAIL),
        all_networks_(),
        wifi_listener_(*this),
        model_listener_list_(nullptr),
        connecting_(false),
        start_scan_(scheduler, [this]() { startScan(); }),
        refresh_current_network_(
            scheduler, [this]() { periodicRefreshCurrentNetwork(); }) {}

  ~Controller() { interface_.removeEventListener(&wifi_listener_); }

  void begin() {
    interface_.addEventListener(&wifi_listener_);
    enabled_ = store_.getIsInterfaceEnabled();
    if (enabled_) notifyEnableChanged();
    std::string ssid = store_.getDefaultSSID();
    if (enabled_ && !ssid.empty()) {
      connect();
    }
  }

  void addListener(Listener* listener) {
    model_listener_list_ = listener->attach(model_listener_list_);
  }

  void removeListener(Listener* listener) {
    model_listener_list_ = listener->detach();
  }

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
    bool started = interface_.startScan();
    if (started) {
      notifyListeners([&](Listener* l) { l->onScanStarted(); });
    }
    return started;
  }

  bool isScanCompleted() const { return interface_.scanCompleted(); }
  bool isEnabled() const { return enabled_; }

  bool isConnecting() const { return connecting_; }

  void toggleEnabled() {
    enabled_ = !enabled_;
    store_.setIsInterfaceEnabled(enabled_);
    if (!enabled_) {
      interface_.disconnect();
    }
    connecting_ = false;
    notifyEnableChanged();
    if (enabled_) {
      resume();
    } else {
      pause();
    }
  }

  void notifyEnableChanged() {
    notifyListeners([&](Listener* l) { l->onEnableChanged(enabled_); });
  }

  void notifyListeners(std::function<void(Listener*)> fn) {
    if (model_listener_list_ != nullptr) {
      auto l = model_listener_list_;
      do {
        fn(l);
        l = l->next_;
      } while (l != model_listener_list_);
    }
  }

  bool getStoredPassword(const std::string& ssid, std::string& passwd) const {
    return store_.getPassword(ssid, passwd);
  }

  void pause() { start_scan_.cancel(); }

  void resume() {
    if (!enabled_) return;
    refreshCurrentNetwork();
    if (!refresh_current_network_.is_scheduled()) {
      refresh_current_network_.scheduleAfter(roo_time::Seconds(2));
    }
    if (interface_.scanCompleted()) {
      notifyListeners([&](Listener* l) { l->onScanCompleted(); });
      start_scan_.scheduleAfter(roo_time::Seconds(15));
    } else {
      startScan();
    }
  }

  void setPassword(const std::string& ssid, const std::string& passwd) {
    store_.setPassword(ssid, passwd);
  }

  bool connect() {
    std::string ssid = store_.getDefaultSSID();
    std::string password;
    store_.getPassword(ssid, password);
    return connect(ssid, password);
  }

  bool connect(const std::string& ssid, const std::string& passwd) {
    std::string default_ssid = store_.getDefaultSSID();
    if (ssid != default_ssid) {
      store_.setDefaultSSID(ssid);
    }
    std::string current_password;
    if (!passwd.empty() && (!store_.getPassword(ssid, current_password) ||
                            current_password != passwd)) {
      store_.setPassword(ssid, passwd);
    }
    if (!interface_.connect(ssid, passwd)) return false;
    connecting_ = true;
    const Network* in_range = lookupNetwork(ssid);
    if (in_range == nullptr) {
      updateCurrentNetwork(ssid, passwd.empty(), -128, WL_DISCONNECTED, true);
    } else {
      updateCurrentNetwork(ssid, in_range->open, in_range->rssi,
                           WL_DISCONNECTED, true);
    }
    return true;
  }

  void disconnect() {
    connecting_ = false;
    interface_.disconnect();
  }

  void forget(const std::string& ssid) {
    store_.clearPassword(ssid);
    if (ssid == store_.getDefaultSSID()) {
      store_.clearDefaultSSID();
    }
  }

 private:
  class WifiListener : public Interface::EventListener {
   public:
    WifiListener(Controller& wifi) : wifi_(wifi) {}

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
    Controller& wifi_;
  };

  friend class WifiListener;

  void onConnectionStateChanged(Interface::EventType type) {
    if (type == Interface::EV_UNKNOWN) return;
    if (type == Interface::EV_DISCONNECTED ||
        type == Interface::EV_CONNECTION_FAILED ||
        type == Interface::EV_CONNECTION_LOST) {
      connecting_ = false;
    }
    updateCurrentNetwork(current_network_.ssid, current_network_.open,
                         current_network_.rssi, getConnectionStatus(type),
                         true);
    notifyListeners([&](Listener* l) { l->onConnectionStateChanged(type); });
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
    if (interface_.getApInfo(&current)) {
      updateCurrentNetwork(std::string((const char*)current.ssid,
                                       strlen((const char*)current.ssid)),
                           (current.authmode == WIFI_AUTH_OPEN), current.rssi,
                           current.status, false);
    } else {
      // Check if we have a default network.
      std::string default_ssid = store_.getDefaultSSID();
      const Network* default_network_in_range = nullptr;
      if (!default_ssid.empty()) {
        // See if the default network is in range according to the latest
        // scan results.
        default_network_in_range = lookupNetwork(default_ssid);
      }
      // Keep erroneous states sticky. Only update if the network has actually
      // changed.
      if (default_network_in_range == nullptr) {
        ConnectionStatus new_status = (default_ssid == current_network_.ssid)
                                          ? current_network_status_
                                          : WL_NO_SSID_AVAIL;
        updateCurrentNetwork(default_ssid, true, -128, new_status, false);
      } else {
        ConnectionStatus new_status = (default_ssid == current_network_.ssid)
                                          ? current_network_status_
                                          : WL_DISCONNECTED;
        updateCurrentNetwork(default_ssid, default_network_in_range->open,
                             default_network_in_range->rssi, new_status, false);
      }
    }
  }

  void updateCurrentNetwork(const std::string& ssid, bool open, int8_t rssi,
                            ConnectionStatus status, bool force_notify) {
    if (!force_notify && rssi == current_network_.rssi &&
        ssid == current_network_.ssid && open == current_network_.open &&
        status == current_network_status_) {
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
    notifyListeners([&](Listener* l) { l->onCurrentNetworkChanged(); });
  }

  void onScanCompleted() {
    current_network_index_ = -1;
    std::vector<NetworkDetails> raw_data;
    interface_.getScanResults(&raw_data, 100);
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
    bool found = false;
    for (uint8_t i = 0; i < dst; ++i) {
      NetworkDetails& src = raw_data[indices[i]];
      Network& dst = all_networks_[i];
      dst.ssid =
          std::string((const char*)src.ssid, strlen((const char*)src.ssid));
      dst.open = (src.authmode == WIFI_AUTH_OPEN);
      dst.rssi = src.rssi;
      if (dst.ssid == current_network_.ssid) {
        found = true;
        current_network_index_ = i;
        if (current_network_status_ == WL_NO_SSID_AVAIL) {
          current_network_status_ = WL_DISCONNECTED;
        }
      }
    }
    if (!found && current_network_status_ == WL_DISCONNECTED) {
      current_network_status_ = WL_NO_SSID_AVAIL;
    }
    notifyListeners([&](Listener* l) { l->onScanCompleted(); });
    if (enabled_) {
      start_scan_.scheduleAfter(roo_time::Seconds(15));
    }
  }

  Store& store_;
  Interface& interface_;
  bool enabled_;
  Network current_network_;
  int16_t current_network_index_;
  ConnectionStatus current_network_status_;
  std::vector<Network> all_networks_;
  WifiListener wifi_listener_;
  Listener* model_listener_list_;
  bool connecting_;

  roo_scheduler::SingletonTask start_scan_;
  roo_scheduler::SingletonTask refresh_current_network_;
};

inline const char* StatusAsString(ConnectionStatus status, bool connecting) {
  return (connecting &&
          (status == WL_DISCONNECTED || status == WL_NO_SSID_AVAIL))
             ? kStrStatusConnecting
         : (status == WL_IDLE_STATUS)     ? kStrStatusConnectedNoInternet
         : (status == WL_NO_SSID_AVAIL)   ? kStrStatusOutOfRange
         : (status == WL_CONNECTED)       ? kStrStatusConnected
         : (status == WL_CONNECT_FAILED)  ? kStrStatusBadPassword
         : (status == WL_CONNECTION_LOST) ? kStrStatusConnectionLost
         : (status == WL_DISCONNECTED)    ? kStrStatusDisconnected
                                          : kStrStatusUnknown;
}

}  // namespace wifi
}  // namespace roo_toolkit
