#include "roo_toolkit/wifi/activity/resources.h"

#include "roo_windows/config.h"

namespace roo_toolkit {
namespace onewire_thermometers {

#if (ROO_WINDOWS_LANG == ROO_WINDOWS_LANG_pl)

const char* kStrThermometers = "Termometry";
const char* kStrThermometerDetails = "Szczegóły termometru";

const char* kStrAssign = "Przypisz";
const char* kStrUnassign = "Rozłącz";

const char* kStrUnassignQuestion = "Usunąć przypisanie?";

const char* kStrUnassignSupportingText =
    "Termometr nie będzie już\n"
    "wskazywał temperatury, i będzie\n"
    "wymagał ponownego przypisania.";

const char* kStrNotAssigned = "(nieprzypisany)";
const char* kStrSelectThermometer = "Przypisz termometr";

#else

const char* kStrThermometers = "Thermometers";
const char* kStrThermometerDetails = "Thermometer details";

const char* kStrAssign = "Assign";
const char* kStrUnassign = "Unassign";

const char* kStrNotAssigned = "(not assigned)";
const char* kStrSelectThermometer = "Assign thermometer";

const char* kStrUnassignQuestion = "Remove assignment?";

const char* kStrUnassignSupportingText =
    "The thermometer will no longer\n"
    "report temperatures, and it will\n"
    "require a re-assignment";

#endif

}  // namespace onewire_thermometers
}  // namespace roo_toolkit
