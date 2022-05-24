#pragma once

#include "roo_toolkit/wifi/network_details.h"

namespace roo_toolkit {
namespace wifi {

class Interface {
 public:
  virtual bool getApInfo(NetworkDetails* info) const = 0;
  virtual bool startScan() = 0;
  virtual bool scanCompleted() const = 0;

  virtual bool getScanResults(std::vector<NetworkDetails>* list,
                              int max_count) const = 0;
  virtual ~Interface() {}
};

}  // namespace wifi
}  // namespace roo_toolkit
