#include <services/ble_watering_service.hpp>

#include <host/ble_hs.h>
#include <nvs.h>

#include "ble_utils.hpp"
#include "ble_watering_service.hpp"

namespace plant {

namespace {

constexpr auto watering_svc_uuid =
    make_uuid128("2f675585-e40a-c088-6941-b245883c4e3a");

uint16_t watering_test_chr_val_handle;
constexpr auto watering_test_chr_uuid =
    make_uuid128("198a6292-be81-4989-bd7d-a408d1b8b08a");

uint16_t watering_schedule_chr_val_handle;
constexpr auto watering_schedule_chr_uuid =
    make_uuid128("3496dac7-7885-4c9c-8e54-0ffebd805485");

} // namespace

BLEWateringService::BLEWateringService()
    : BLEService{
          BLE_GATT_SVC_TYPE_PRIMARY, &watering_svc_uuid.u,
          std::array{ble_gatt_chr_def{
                         .uuid = &watering_test_chr_uuid.u,
                         .access_cb = watering_chr_access,
                         .arg = this,
                         .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
                         .val_handle = &watering_test_chr_val_handle},
                     ble_gatt_chr_def{
                         .uuid = &watering_schedule_chr_uuid.u,
                         .access_cb = watering_chr_access,
                         .arg = this,
                         .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
                         .val_handle = &watering_schedule_chr_val_handle}}} {
}

void BLEWateringService::service_added() {
    watering_schedule() = WateringSchedule{};
    watering_schedule().read_from_storage();
}

int BLEWateringService::watering_chr_access(uint16_t conn_handle,
                                            uint16_t attr_handle,
                                            ble_gatt_access_ctxt* ctxt,
                                            void* arg) {

    auto& self = *static_cast<BLEWateringService*>(arg);

    auto error_handler = [ctxt]() {
        ESP_LOGE("BLEWateringService",
                 "unexpected access operation to heart rate "
                 "characteristic, opcode: %d",
                 ctxt->op);
        return BLE_ATT_ERR_UNLIKELY;
    };

    /* Handle access events */
    /* Note: Heart rate characteristic is read only */
    switch (ctxt->op) {

    /* Read characteristic event */
    case BLE_GATT_ACCESS_OP_READ_CHR:
        /* Verify connection handle */
        if (conn_handle != BLE_HS_CONN_HANDLE_NONE) {
            ESP_LOGI("BLEWateringService",
                     "characteristic read; conn_handle=%d attr_handle=%d",
                     conn_handle, attr_handle);
        } else {
            ESP_LOGI("BLEWateringService",
                     "characteristic read by nimble stack; attr_handle=%d",
                     attr_handle);
        }

        /* Verify attribute handle */
        if (attr_handle == watering_test_chr_val_handle) {
            int res = os_mbuf_append(ctxt->om, &self.watering_test(),
                                     WateringTest::size);
            return res == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        } else if (attr_handle == watering_schedule_chr_val_handle) {
            int res = os_mbuf_append(ctxt->om, &self.watering_schedule(),
                                     WateringSchedule::size);
            return res == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        } else {
            return error_handler();
        }

    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        if (conn_handle != BLE_HS_CONN_HANDLE_NONE) {
            ESP_LOGI("BLEWateringService",
                     "Characteristic write; conn_handle=%d attr_handle=%d",
                     conn_handle, attr_handle);
        } else {
            ESP_LOGI("BLEWateringService",
                     "Characteristic write by NimBLE stack; attr_handle=%d",
                     attr_handle);
        }
        if (attr_handle == watering_schedule_chr_val_handle) {
            constexpr auto packet_size = WateringSchedule::size;
            const auto bytes_received = OS_MBUF_PKTLEN(ctxt->om);
            if (bytes_received != packet_size) {
                return error_handler();
            }

            uint16_t bytes_written{};
            WateringSchedule schedule{};
            auto rc = gatt_svr_write(ctxt->om, packet_size, packet_size,
                                     &schedule, &bytes_written);

            if (rc != 0) {
                ESP_LOGI("BLEWateringService", "Cannot write time to memory\n");
                return rc;
            }

            self.watering_schedule() = schedule;
            self.watering_schedule().write_to_storage();

            ble_gatts_chr_updated(attr_handle);

            ESP_LOGI("BLEWateringService",
                     "New program: %lld / %lu / %u / %u / %d\n",
                     self.watering_schedule().start_time,
                     self.watering_schedule().watering_period,
                     self.watering_schedule().watering_duration,
                     self.watering_schedule().flow_speed,
                     self.watering_schedule().enabled);
            ESP_LOGI("BLEWateringService",
                     "Notification/Indication scheduled for "
                     "all subscribed peers.\n");
            return rc;
        } else if (attr_handle == watering_test_chr_val_handle) {
            constexpr auto packet_size = WateringTest::size;
            const auto bytes_received = OS_MBUF_PKTLEN(ctxt->om);
            if (bytes_received != packet_size) {
                return error_handler();
            }

            uint16_t bytes_written{};
            WateringTest test{};
            auto rc = gatt_svr_write(ctxt->om, packet_size, packet_size, &test,
                                     &bytes_written);

            if (rc != 0) {
                ESP_LOGI("BLEWateringService", "Cannot write test to memory\n");
                return rc;
            }

            self.watering_test() = test;

            ble_gatts_chr_updated(attr_handle);

            ESP_LOGI("BLEWateringService", "New test request: %u @ %u\n",
                     self.watering_test().duration,
                     self.watering_test().flow_speed);
            ESP_LOGI("BLEWateringService",
                     "Notification/Indication scheduled for "
                     "all subscribed peers.\n");
            return rc;
        }
        return error_handler();

    /* Unknown event */
    default:
        return error_handler();
    }

    return 0;
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
