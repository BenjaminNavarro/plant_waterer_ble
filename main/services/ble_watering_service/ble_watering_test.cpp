#include <services/ble_watering_service/ble_watering_test.hpp>
#include <services/ble_utils.hpp>

#include <host/ble_hs.h>
#include <host/ble_gatt.h>
#include <nvs.h>

#include <sys/time.h>
#include <cstring>

namespace plant {

namespace {

constexpr auto watering_test_chr_uuid =
    make_uuid128("198a6292-be81-4989-bd7d-a408d1b8b08a");

} // namespace

BLEWateringTestCharacteristic::BLEWateringTestCharacteristic()
    : BLECharacteristic{"BLEWateringTest", &watering_test_chr_uuid.u,
                        BLE_GATT_CHR_F_WRITE, WateringTest::size} {
}

void BLEWateringTestCharacteristic::on_write(std::span<const std::byte> memory) {
    std::memcpy(&test_, memory.data(), memory.size());

    ESP_LOGI("BLEWateringService", "New test request: %u @ %u\n",
             test().duration, test().flow_speed);
}

} // namespace plant
