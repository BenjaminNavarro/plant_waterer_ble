#include <services/ble_name_service.hpp>

#include <host/ble_hs.h>

#include "ble_utils.hpp"
#include "gap.hpp"

#include <algorithm>

namespace plant {

namespace {

#define BLE_UUID128_INIT_R(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12,  \
                           _13, _14, _15, _16)                                 \
    BLE_UUID128_INIT(_1, _15, _14, _13, _12, _11, _10, _9, _8, _7, _6, _5, _4, \
                     _3, _2, _1)

/* Custom name service */
constexpr auto name_svc_uuid =
    make_uuid128("4f736c21-2054-4786-93fe-a5c4b028dbef");

uint16_t name_chr_val_handle;
const auto name_chr_uuid = make_uuid128("b8b4c3af-fa31-4de4-9fa1-a26ea5da7f0b");

} // namespace

BLENameService::BLENameService()
    : BLEService{BLE_GATT_SVC_TYPE_PRIMARY, &name_svc_uuid.u,
                 std::array{ble_gatt_chr_def{
                     .uuid = &name_chr_uuid.u,
                     .access_cb = name_chr_access,
                     .arg = this,
                     .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
                     .val_handle = &name_chr_val_handle}}} {
}

int BLENameService::name_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                                    ble_gatt_access_ctxt* ctxt, void* arg) {

    auto& self = *static_cast<BLENameService*>(arg);

    auto error_handler = [ctxt]() {
        ESP_LOGE("BLENameService",
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
            ESP_LOGI("BLENameService",
                     "characteristic read; conn_handle=%d attr_handle=%d",
                     conn_handle, attr_handle);
        } else {
            ESP_LOGI("BLENameService",
                     "characteristic read by nimble stack; attr_handle=%d",
                     attr_handle);
        }

        /* Verify attribute handle */
        if (attr_handle == name_chr_val_handle) {
            const auto name = self.name().as_array();
            int res = os_mbuf_append(ctxt->om, name.data(), name.size());
            return res == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        }
        return error_handler();

    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        if (conn_handle != BLE_HS_CONN_HANDLE_NONE) {
            ESP_LOGI("BLENameService",
                     "Characteristic write; conn_handle=%d attr_handle=%d",
                     conn_handle, attr_handle);
        } else {
            ESP_LOGI("BLENameService",
                     "Characteristic write by NimBLE stack; attr_handle=%d",
                     attr_handle);
        }
        if (attr_handle == name_chr_val_handle) {
            std::array<char, ManufacturerData::Part2::name_max_size + 1> name;
            uint16_t length{};

            name.fill(0);

            const auto om_len = OS_MBUF_PKTLEN(ctxt->om);

            auto rc =
                gatt_svr_write(ctxt->om, 0, name.size(), name.data(), &length);
            ESP_LOGI("BLENameService", "%d byes received, %d bytes copied\n",
                     om_len, length);
            if (rc != 0) {
                ESP_LOGI("BLENameService", "Cannot write name to memory\n");
                return rc;
            }

            self.name().set(name);
            ble_gatts_chr_updated(attr_handle);
            write_user_defined_name_to_storage(self.name());

            {
                auto mfg_data = getCurrentManufacturerData();
                mfg_data.part2.user_defined_name = self.name().as_array();
                updateManufacturerData(mfg_data);
            }

            ESP_LOGI("BLENameService", "New device name: %s\n",
                     self.name().get().data());
            ESP_LOGI("BLENameService", "Notification/Indication scheduled for "
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