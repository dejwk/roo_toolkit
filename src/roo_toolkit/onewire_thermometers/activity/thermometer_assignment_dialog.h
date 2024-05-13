#pragma once

#include <string>

#include "roo_onewire/thermometer_roles.h"
#include "roo_windows/composites/radio/radio_list.h"
#include "roo_windows/containers/list_layout.h"
#include "roo_windows/dialogs/dialog.h"
#include "roo_windows/dialogs/radio_list_dialog.h"

namespace roo_toolkit {
namespace onewire_thermometers {

class UnassignedThermometerRadioGroupItem
    : public roo_windows::HorizontalLayout {
 public:
  UnassignedThermometerRadioGroupItem(const roo_windows::Environment& env);

  UnassignedThermometerRadioGroupItem(
      const UnassignedThermometerRadioGroupItem& other);

  void set(std::string rom_code, std::string reading);

 private:
  roo_windows::TextLabel rom_code_;
  roo_windows::TextLabel reading_;
};

class UnassignedThermometerRadioGroupModel
    : public roo_windows::ListModel<UnassignedThermometerRadioGroupItem> {
 public:
  UnassignedThermometerRadioGroupModel(roo_onewire::ThermometerRoles& model)
      : model_(model) {}

  int elementCount() const override;
  void set(int idx, UnassignedThermometerRadioGroupItem& dest) const override;

 private:
  roo_onewire::ThermometerRoles& model_;
};

class UnassignedThermometerSelectionDialog
    : public roo_windows::RadioListDialog<UnassignedThermometerRadioGroupModel>,
      public roo_onewire::ThermometerRoles::EventListener {
 public:
  UnassignedThermometerSelectionDialog(const roo_windows::Environment& env,
                                       roo_onewire::ThermometerRoles& model);

  void onEnter() override;
  void onExit(int result) override;

  void onChange() override;

  void discoveryCompleted() override;
  void conversionCompleted() override;

 private:
  roo_onewire::ThermometerRoles& model_;
  UnassignedThermometerRadioGroupModel list_model_;
  roo_onewire::RomCode selected_rom_code_;
};

}  // namespace onewire_thermometers
}  // namespace roo_toolkit