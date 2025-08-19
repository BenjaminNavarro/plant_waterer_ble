#include <services/ble_current_time_service.hpp>

#include "host/ble_hs.h"
#include "ble_utils.hpp"

#include <sys/time.h>

namespace plant {

namespace {

/* Current time service */
const auto current_time_svc_uuid =
    make_uuid128("87f4d02e-698f-4c46-91f2-5f714c877b0a");

uint16_t current_time_chr_val_handle;
const auto current_time_chr_uuid =
    make_uuid128("21bc4af5-44f0-4a7b-aa36-a110a0ac0ad2");

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
                 "unexpected access operation, opcode: %d", ctxt->op);
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
            struct timeval now_tv;
            gettimeofday(&now_tv, nullptr);

            int res =
                os_mbuf_append(ctxt->om, &now_tv.tv_sec, sizeof(now_tv.tv_sec));
            return res == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        }
        return error_handler();

    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        if (conn_handle != BLE_HS_CONN_HANDLE_NONE) {
            ESP_LOGI("BLECurrentTimeService",
                     "Characteristic write; conn_handle=%d attr_handle=%d",
                     conn_handle, attr_handle);
        } else {
            ESP_LOGI("BLECurrentTimeService",
                     "Characteristic write by NimBLE stack; attr_handle=%d",
                     attr_handle);
        }
        if (attr_handle == current_time_chr_val_handle) {
            constexpr auto packet_size = sizeof(std::int64_t);
            const auto bytes_received = OS_MBUF_PKTLEN(ctxt->om);
            if (bytes_received != packet_size) {
                return error_handler();
            }

            uint16_t bytes_written{};
            std::int64_t current_time{};
            auto rc = gatt_svr_write(ctxt->om, packet_size, packet_size,
                                     &current_time, &bytes_written);

            if (rc != 0) {
                ESP_LOGI("BLECurrentTimeService",
                         "Cannot write time to memory\n");
                return rc;
            }

            struct timeval tv;
            tv.tv_sec = current_time;
            tv.tv_usec = 0;
            settimeofday(&tv, nullptr);

            ble_gatts_chr_updated(attr_handle);

            ESP_LOGI("BLECurrentTimeService", "New time: %lld\n", current_time);
            ESP_LOGI("BLECurrentTimeService",
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

} // namespace plant