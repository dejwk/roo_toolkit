#pragma once

#include <Arduino.h>

#include "roo_toolkit/onewire_thermometers/activity/list_activity.h"
#include "roo_toolkit/onewire_thermometers/activity/thermometer_assignment_dialog.h"
#include "roo_toolkit/onewire_thermometers/activity/thermometer_details_activity.h"

namespace roo_toolkit {
namespace onewire_thermometers {

class Configurator {
 public:
  Configurator(const roo_windows::Environment& env,
               roo_onewire::ThermometerRoles& model)
      : model_(model),
        list_(env, env.scheduler(), model_,
              [this](roo_windows::Task& task, int id) {
                thermometerSelected(task, id);
              }),
        details_(
            env, model_,
            [this](roo_windows::Task& task, int id) {
              assignThermometer(task, id);
            },
            [this](roo_windows::Task& task, int id) {
              unassignThermometer(task, id);
            }),
        assignment_(env, model_) {}

  roo_windows::Activity& main() { return list_; }

  void thermometerSelected(roo_windows::Task& task, int id) {
    if (model_.thermometerRoleById(id).isAssigned()) {
      details_.enter(task, id);
    } else {
      assignThermometer(task, id);
    }
  }

  void assignThermometer(roo_windows::Task& task, int id) {
    task.showDialog(assignment_, [&task, this, id](int idx) {
      if (idx == 1) {
        model_.assign(id, model_.unassigned()[assignment_.selected()]);
      }
    });
  }

  void unassignThermometer(roo_windows::Task& task, int id) {
    task.showAlertDialog(
        kStrUnassignQuestion, kStrUnassignSupportingText,
        {roo_windows::kStrDialogCancel, roo_windows::kStrDialogOK},
        [this, id](int idx) {
          if (idx == 1) {
            model_.unassign(id);
          }
        });
  }

  roo_onewire::ThermometerRoles& model_;
  ListActivity list_;
  ThermometerDetailsActivity details_;
  UnassignedThermometerSelectionDialog assignment_;
};

}  // namespace onewire_thermometers
}  // namespace roo_toolkit
