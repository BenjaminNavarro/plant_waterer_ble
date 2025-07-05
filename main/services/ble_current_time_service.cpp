#include <services/ble_current_time_service.hpp>

#include "host/ble_hs.h"

namespace plant {

namespace {

/* Current time service */
const ble_uuid16_t current_time_svc_uuid = BLE_UUID16_INIT(0x1805);

uint32_t current_time_chr_val = {0};
uint16_t current_time_chr_val_handle;
const ble_uuid16_t current_time_chr_uuid = BLE_UUID16_INIT(0x2A2B);

uint16_t current_time_chr_conn_handle = 0;
bool current_time_chr_conn_handle_inited = false;
bool current_time_ind_status = false;

} // namespace

BLECurrentTimeService::BLECurrentTimeService()
    : BLEService{BLE_GATT_SVC_TYPE_PRIMARY, &current_time_svc_uuid.u,
                 std::array{ble_gatt_chr_def{
                     .uuid = &current_time_chr_uuid.u,
                     .access_cb = current_time_chr_access,
                     .arg = this,
                     .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
                     .val_handle = &current_time_chr_val_handle}}} {
}

int BLECurrentTimeService::current_time_chr_access(uint16_t conn_handle,
                                                   uint16_t attr_handle,
                                                   ble_gatt_access_ctxt* ctxt,
                                                   void* arg) {

    auto& self = *static_cast<BLECurrentTimeService*>(arg);

    auto error_handler = [ctxt]() {
        ESP_LOGE("BLECurrentTimeService",
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
            ESP_LOGI("BLECurrentTimeService",
                     "characteristic read; conn_handle=%d attr_handle=%d",
                     conn_handle, attr_handle);
        } else {
            ESP_LOGI("BLECurrentTimeService",
                     "characteristic read by nimble stack; attr_handle=%d",
                     attr_handle);
        }

        /* Verify attribute handle */
        if (attr_handle == current_time_chr_val_handle) {
            self.current_time().seconds =
                (self.current_time().seconds +
                 xTaskGetTickCount() / xPortGetTickRateHz()) %
                60;
            int res = os_mbuf_append(ctxt->om, &self.current_time(),
                                     sizeof(self.current_time()));
            return res == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        }
        return error_handler();

    // TODO handle write

    /* Unknown event */
    default:
        return error_handler();
    }

    return 0;
}

} // namespace plant