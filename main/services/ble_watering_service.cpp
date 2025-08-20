#include <services/ble_watering_service.hpp>

#include <host/ble_hs.h>
#include <nvs.h>

#include "ble_utils.hpp"
#include "host/ble_gatt.h"
#include "ble_watering_service.hpp"

#include <sys/time.h>
#include <cstring>

namespace plant {

namespace {

constexpr auto watering_svc_uuid =
    make_uuid128("2f675585-e40a-c088-6941-b245883c4e3a");

constexpr auto watering_schedule_chr_uuid =
    make_uuid128("3496dac7-7885-4c9c-8e54-0ffebd805485");

constexpr auto watering_test_chr_uuid =
    make_uuid128("198a6292-be81-4989-bd7d-a408d1b8b08a");

constexpr auto watering_state_chr_uuid =
    make_uuid128("ed4cb13c-71cc-460b-a781-5530878f7aa5");

} // namespace

BLEWateringScheduleCharacteristic::BLEWateringScheduleCharacteristic()
    : BLECharacteristic{"BLEWateringSchedule", &watering_schedule_chr_uuid.u,
                        BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
                        WateringSchedule::size} {
}

std::span<const std::byte>
BLEWateringScheduleCharacteristic::on_read_access(std::span<std::byte> memory) {
    std::memcpy(memory.data(), &schedule_, WateringSchedule::size);
    return memory;
}

void BLEWateringScheduleCharacteristic::on_write(
    std::span<const std::byte> memory) {
    std::memcpy(&schedule_, memory.data(), memory.size());
    schedule().write_to_storage();
    ESP_LOGI("BLEWateringService", "New program: %lld / %lu / %u / %u / %d\n",
             schedule().start_time, schedule().watering_period,
             schedule().watering_duration, schedule().flow_speed,
             schedule().enabled);
}

BLEWateringTestCharacteristic::BLEWateringTestCharacteristic()
    : BLECharacteristic{"BLEWateringTest", &watering_test_chr_uuid.u,
                        BLE_GATT_CHR_F_WRITE, WateringTest::size} {
}

void BLEWateringTestCharacteristic::on_write(std::span<const std::byte> memory) {
    std::memcpy(&test_, memory.data(), memory.size());

    ESP_LOGI("BLEWateringService", "New test request: %u @ %u\n",
             test().duration, test().flow_speed);
}

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

BLEWateringService::BLEWateringService()
    : BLEServiceWrapper{BLE_GATT_SVC_TYPE_PRIMARY, &watering_svc_uuid.u} {
}

void BLEWateringService::service_added() {
    watering_schedule().schedule() = WateringSchedule{};
    watering_schedule().schedule().read_from_storage();
}

void WateringSchedule::read_from_storage() {
    nvs_handle_t nvs_handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &nvs_handle));

    // Read the size of memory space required for blob
    size_t required_size = 0; // value will default to 0, if not set yet in NVS
    esp_err_t err =
        nvs_get_blob(nvs_handle, "schedule", nullptr, &required_size);
    ESP_ERROR_CHECK(err == ESP_OK or err == ESP_ERR_NVS_NOT_FOUND ? ESP_OK
                                                                  : ESP_FAIL);
    if (required_size != WateringSchedule::size) {
        nvs_close(nvs_handle);

        ESP_LOGI("WateringSchedule",
                 "Incorrect size, overwriting with default value\n");

        const auto default_schedule = WateringSchedule{};
        default_schedule.write_to_storage();
        read_from_storage();
        return;
    }

    WateringSchedule schedule;
    ESP_ERROR_CHECK(
        nvs_get_blob(nvs_handle, "schedule", &schedule, &required_size));

    nvs_close(nvs_handle);

    *this = schedule;
}

void WateringSchedule::write_to_storage() const {
    nvs_handle_t nvs_handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &nvs_handle));

    ESP_ERROR_CHECK(
        nvs_set_blob(nvs_handle, "schedule", this, WateringSchedule::size));

    ESP_ERROR_CHECK(nvs_commit(nvs_handle));

    nvs_close(nvs_handle);
}

} // namespace plant
