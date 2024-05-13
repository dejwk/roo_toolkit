#include "roo_toolkit/onewire_thermometers/activity/list_activity.h"

#include "roo_toolkit/onewire_thermometers/activity/resources.h"

using roo_onewire::ThermometerRoles;

namespace roo_toolkit {
namespace onewire_thermometers {

ThermometerListItem::ThermometerListItem(const roo_windows::Environment& env,
                                         ThermometerSelectedFn on_click)
    : HorizontalLayout(env),
      thermometer_icon_(env, SCALED_ROO_ICON(filled, device_thermostat)),
      name_(env, "", roo_windows::font_subtitle1()),
      reading_(env, "", roo_windows::font_subtitle1()),
      on_click_(on_click) {
  setGravity(roo_windows::Gravity(roo_windows::kHorizontalGravityNone,
                                  roo_windows::kVerticalGravityMiddle));
  add(thermometer_icon_);

  name_.setMargins(roo_windows::MARGIN_NONE);
  name_.setPadding(roo_windows::PADDING_TINY);
  add(name_, HorizontalLayout::Params().setWeight(1));

  // reading_.setMargins(roo_windows::MARGIN_NONE);
  // reading_.setPadding(roo_windows::PADDING_REGULAR, PADDING_TINY);
  add(reading_, HorizontalLayout::Params().setWeight(0));
}

ThermometerListItem::ThermometerListItem(const ThermometerListItem& other)
    : HorizontalLayout(other),
      thermometer_icon_(other.thermometer_icon_),
      name_(other.name_),
      reading_(other.reading_),
      on_click_(other.on_click_) {
  add(thermometer_icon_);
  add(name_, HorizontalLayout::Params().setWeight(1));
  add(reading_, HorizontalLayout::Params().setWeight(1));
}

// Sets this item to show the specified network.
void ThermometerListItem::set(const roo_onewire::ThermometerRole& thermometer) {
  id_ = thermometer.id();
  name_.setText(thermometer.name());
  if (!thermometer.isAssigned() ||
      thermometer.readTemperature().value.isUnknown()) {
    reading_.setText("");
  } else {
    reading_.setTextf("%3.1f°C",
                      thermometer.readTemperature().value.degCelcius());
  }
  thermometer_icon_.setVisibility(thermometer.isAssigned() ? VISIBLE
                                                           : INVISIBLE);
}

ThermometerListModel::ThermometerListModel(roo_onewire::ThermometerRoles& model)
    : model_(model) {}

int ThermometerListModel::elementCount() const {
  return model_.thermometer_roles_count();
}

void ThermometerListModel::set(int idx, ThermometerListItem& dest) const {
  dest.set(model_.thermometer_role(idx));
}

ListActivity::ListActivity(const roo_windows::Environment& env,
                           roo_scheduler::Scheduler& scheduler,
                           ThermometerRoles& model,
                           ThermometerSelectedFn thermometer_selected_fn)
    : model_(model),
      contents_(env, model, thermometer_selected_fn),
      scrollable_container_(env, contents_),
      refresh_task_(
          scheduler, [this]() { model_.update(); }, roo_time::Millis(1000)) {}

ListActivityContents::ListActivityContents(
    const roo_windows::Environment& env, ThermometerRoles& model,
    ThermometerSelectedFn thermometer_selected_fn)
    : VerticalLayout(env),
      model_(model),
      title_(env, kStrThermometers),
      list_model_(model),
      list_(env, list_model_,
            ThermometerListItem(env, thermometer_selected_fn)) {
  add(title_);
  add(list_, VerticalLayout::Params());
}

void ListActivityContents::conversionCompleted() { list_.modelChanged(); }

void ListActivity::onStart() {
  model_.addEventListener(&contents_);
  refresh_task_.start();
}

void ListActivity::onStop() {
  refresh_task_.stop();
  model_.removeEventListener(&contents_);
}

}  // namespace onewire_thermometers
}  // namespace roo_toolkit
