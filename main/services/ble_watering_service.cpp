#include <services/ble_watering_service.hpp>

#include "host/ble_hs.h"

namespace plant {

namespace {

// 2f675585-e40a-c088-6941-b245883c4e3a
const ble_uuid128_t watering_svc_uuid =
    BLE_UUID128_INIT(0x3a, 0x4e, 0x3c, 0x88, 0x45, 0xb2, 0x41, 0x69, 0x88, 0xc0,
                     0x0a, 0xe4, 0x85, 0x55, 0x67, 0x2f);

uint16_t watering_test_chr_val_handle;
const ble_uuid128_t watering_test_chr_uuid =
    BLE_UUID128_INIT(0xce, 0xe6, 0x55, 0x0c, 0x9b, 0xf3, 0x46, 0xd4, 0xb9, 0xda,
                     0x43, 0x4d, 0xc8, 0x52, 0xe7, 0xd2);

uint16_t watering_schedule_chr_val_handle;
const ble_uuid128_t watering_schedule_chr_uuid =
    BLE_UUID128_INIT(0x28, 0xeb, 0x73, 0x8b, 0x31, 0x26, 0x4c, 0x9a, 0x80, 0xc8,
                     0x1f, 0x3f, 0x58, 0x11, 0xca, 0xbc);

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
                                     sizeof(WateringTest));
            return res == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        } else if (attr_handle == watering_schedule_chr_val_handle) {
            int res = os_mbuf_append(ctxt->om, &self.watering_schedule(),
                                     sizeof(WateringSchedule));
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