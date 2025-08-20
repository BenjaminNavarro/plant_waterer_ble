#include <services/ble_watering_service/ble_watering_service.hpp>
#include <services/ble_utils.hpp>

#include <host/ble_hs.h>
#include <nvs.h>

#include "host/ble_gatt.h"

#include <sys/time.h>
#include <cstring>

namespace plant {

namespace {

constexpr auto watering_svc_uuid =
    make_uuid128("2f675585-e40a-c088-6941-b245883c4e3a");

} // namespace

BLEWateringService::BLEWateringService()
    : BLEServiceWrapper{BLE_GATT_SVC_TYPE_PRIMARY, &watering_svc_uuid.u} {
}

void BLEWateringService::service_added() {
    watering_schedule().schedule() = WateringSchedule{};
    watering_schedule().schedule().read_from_storage();
}

} // namespace plant
