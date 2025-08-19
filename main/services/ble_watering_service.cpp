#include <services/ble_watering_service.hpp>

#include "host/ble_hs.h"

#include "ble_utils.hpp"

namespace plant {

namespace {

//
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

    /* Unknown event */
    default:
        return error_handler();
    }

    return 0;
}

} // namespace plant