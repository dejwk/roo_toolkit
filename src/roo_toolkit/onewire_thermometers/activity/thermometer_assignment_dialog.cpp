#include "roo_toolkit/onewire_thermometers/activity/thermometer_assignment_dialog.h"

#include "roo_display/ui/string_printer.h"
#include "roo_toolkit/onewire_thermometers/activity/resources.h"

using roo_windows::Dialog;

using roo_onewire::ThermometerRoles;

namespace roo_toolkit {
namespace onewire_thermometers {

UnassignedThermometerRadioGroupItem::UnassignedThermometerRadioGroupItem(
    const roo_windows::Environment& env)
    : HorizontalLayout(env),
      rom_code_(env, "1234567812345678", roo_windows::font_subtitle1()),
      reading_(env, "00.0°C", roo_windows::font_subtitle1()) {
  setGravity(roo_windows::Gravity(roo_windows::kHorizontalGravityNone,
                                  roo_windows::kVerticalGravityMiddle));
  rom_code_.setMargins(roo_windows::MARGIN_NONE);
  rom_code_.setPadding(roo_windows::PADDING_SMALL);
  add(rom_code_, HorizontalLayout::Params().setWeight(1));
  add(reading_);
}

UnassignedThermometerRadioGroupItem::UnassignedThermometerRadioGroupItem(
    const UnassignedThermometerRadioGroupItem& other)
    : HorizontalLayout(other),
      rom_code_(other.rom_code_),
      reading_(other.reading_) {
  add(rom_code_);
  add(reading_);
}

void UnassignedThermometerRadioGroupItem::set(std::string rom_code,
                                              std::string reading) {
  rom_code_.setText(std::move(rom_code));
  reading_.setText(std::move(reading));
}

int UnassignedThermometerRadioGroupModel::elementCount() const {
  return model_.unassigned().size();
}

void UnassignedThermometerRadioGroupModel::set(
    int idx, UnassignedThermometerRadioGroupItem& dest) const {
  roo_onewire::RomCode rom_code = model_.unassigned()[idx];
  char rom_code_text[17];
  rom_code_text[16] = 0;
  model_.unassigned()[idx].toCharArray(rom_code_text);
  roo_temperature::Temperature temp = model_.temperatureByRomCode(rom_code);
  std::string reading;
  if (temp.isUnknown() || temp >= roo_temperature::DegCelcius(85) ||
      temp <= roo_temperature::DegCelcius(-55)) {
    reading = "";
  } else {
    reading = roo_display::StringPrintf("%2.1f°C", temp.degCelcius());
  }
  dest.set(rom_code_text, std::move(reading));
}

UnassignedThermometerSelectionDialog::UnassignedThermometerSelectionDialog(
    const roo_windows::Environment& env, ThermometerRoles& model)
    : roo_windows::RadioListDialog<UnassignedThermometerRadioGroupModel>(env),
      model_(model),
      list_model_(model) {
  setTitle(kStrSelectThermometer);
  setModel(list_model_);
}

void UnassignedThermometerSelectionDialog::onEnter() {
  model_.addEventListener(this);
  model_.update();
  reset();
}

void UnassignedThermometerSelectionDialog::onExit(int result) {
  model_.removeEventListener(this);
}

void UnassignedThermometerSelectionDialog::discoveryCompleted() {
  int s = selected();
  int new_s = -1;
  if (s >= 0) {
    // Update the selection to stick to the originally selected rom code.
    for (int i = 0; i < model_.unassigned().size(); ++i) {
      if (model_.unassigned()[i] == selected_rom_code_) {
        // Found!
        new_s = i;
        break;
      }
    }
  }
  contentsChanged();
  setSelected(new_s);
}

void UnassignedThermometerSelectionDialog::conversionCompleted() {
  contentsChanged();
}

void UnassignedThermometerSelectionDialog::onChange() {
  int s = selected();
  selected_rom_code_ =
      (s >= 0) ? model_.unassigned()[s] : roo_onewire::RomCode();
  RadioListDialog::onChange();
}

}  // namespace onewire_thermometers
}  // namespace roo_toolkit
