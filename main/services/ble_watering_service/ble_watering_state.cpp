#include <services/ble_watering_service/ble_watering_state.hpp>
#include <services/ble_utils.hpp>

#include <host/ble_hs.h>
#include <nvs.h>

#include "host/ble_gatt.h"

#include <sys/time.h>
#include <cstring>

namespace plant {

namespace {

constexpr auto watering_state_chr_uuid =
    make_uuid128("ed4cb13c-71cc-460b-a781-5530878f7aa5");

} // namespace

BLEWateringStateCharacteristic::BLEWateringStateCharacteristic()
    : BLECharacteristic{"BLEWateringState", &watering_state_chr_uuid.u,
                        BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY} {
}

std::span<const std::byte>
BLEWateringStateCharacteristic::on_read_access(std::span<std::byte> memory) {
    std::memcpy(memory.data(), &state_, WateringState::size);
    return memory;
}

void BLEWateringStateCharacteristic::notify_watering_state_change() {
    ESP_LOGI("BLEWateringService",
             "Sending watering state change notification");
    send_update_notification();
}

} // namespace plant
