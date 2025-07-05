#include <services/ble_battery_service.hpp>

#include "host/ble_hs.h"

namespace plant {

namespace {

/* Current time service */
const ble_uuid16_t battery_svc_uuid = BLE_UUID16_INIT(0x180F);

uint32_t battery_chr_val = {0};
uint16_t battery_chr_val_handle;
const ble_uuid16_t battery_level_chr_uuid = BLE_UUID16_INIT(0x2A19);

uint16_t battery_chr_conn_handle = 0;
bool battery_chr_conn_handle_inited = false;
bool battery_ind_status = false;

} // namespace

BLEBatteryService::BLEBatteryService()
    : BLEService{BLE_GATT_SVC_TYPE_PRIMARY, &battery_svc_uuid.u,
                 std::array{
                     ble_gatt_chr_def{.uuid = &battery_level_chr_uuid.u,
                                      .access_cb = battery_level_chr_access,
                                      .arg = this,
                                      .flags = BLE_GATT_CHR_F_READ,
                                      .val_handle = &battery_chr_val_handle}}},
      battery_level_{0.9f} {
}

int BLEBatteryService::battery_level_chr_access(uint16_t conn_handle,
                                                uint16_t attr_handle,
                                                ble_gatt_access_ctxt* ctxt,
                                                void* arg) {

    auto& self = *static_cast<BLEBatteryService*>(arg);

    auto error_handler = [ctxt]() {
        ESP_LOGE("BLEBatteryService",
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
            ESP_LOGI("BLEBatteryService",
                     "characteristic read; conn_handle=%d attr_handle=%d",
                     conn_handle, attr_handle);
        } else {
            ESP_LOGI("BLEBatteryService",
                     "characteristic read by nimble stack; attr_handle=%d",
                     attr_handle);
        }

        /* Verify attribute handle */
        if (attr_handle == battery_chr_val_handle) {
            uint8_t battery_value = self.battery_level() * 100.f;
            self.battery_level() *= 0.9;
            int res =
                os_mbuf_append(ctxt->om, &battery_value, sizeof(battery_value));
            return res == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        }
        return error_handler();

    /* Unknown event */
    default:
        return error_handler();
    }

    return 0;
}

} // namespace plant