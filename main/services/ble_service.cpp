#include <services/ble_service.hpp>

#include <esp_log.h>
#include <host/ble_hs.h>
#include <services/gatt/ble_svc_gatt.h>

#include "ble_service.hpp"
#include "ble_utils.hpp"

namespace plant {

int plant::BLECharacteristicBase::on_access(uint16_t conn_handle,
                                            uint16_t attr_handle,
                                            ble_gatt_access_ctxt* ctxt,
                                            void* arg) {
    auto& self = *static_cast<BLECharacteristicBase*>(arg);

    const auto* name = self.name().data();

    auto error_handler = [ctxt, name]() {
        ESP_LOGE(name, "unexpected access operation, opcode: %d", ctxt->op);
        return BLE_ATT_ERR_UNLIKELY;
    };

    /* Handle access events */
    switch (ctxt->op) {

    /* Read characteristic event */
    case BLE_GATT_ACCESS_OP_READ_CHR:
        /* Verify connection handle */
        if (conn_handle != BLE_HS_CONN_HANDLE_NONE) {
            ESP_LOGI(name, "characteristic read; conn_handle=%d attr_handle=%d",
                     conn_handle, attr_handle);
        } else {
            ESP_LOGI(name, "characteristic read by nimble stack; attr_handle=%d",
                     attr_handle);
        }

        /* Verify attribute handle */
        if (attr_handle == self.characteristic_value_handle_) {
            const auto span = self.on_read_access(self.read_memory_);
            // Span must point to data inside memory_
            if (span.data() == nullptr or span.size() == 0 or
                span.data() != self.read_memory_.data() or
                span.size() > self.read_memory_.size()) {
                return BLE_ATT_ERR_READ_NOT_PERMITTED;
            }

            int res = os_mbuf_append(ctxt->om, span.data(), span.size());
            if (res == 0) {
                self.on_read();
                return 0;
            } else {
                return BLE_ATT_ERR_INSUFFICIENT_RES;
            }
        }
        return error_handler();

    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        if (conn_handle != BLE_HS_CONN_HANDLE_NONE) {
            ESP_LOGI(name, "Characteristic write; conn_handle=%d attr_handle=%d",
                     conn_handle, attr_handle);
        } else {
            ESP_LOGI(name,
                     "Characteristic write by NimBLE stack; attr_handle=%d",
                     attr_handle);
        }
        if (attr_handle == self.characteristic_value_handle_) {
            const auto om_len = OS_MBUF_PKTLEN(ctxt->om);

            if (self.min_packet_size_ > self.write_memory_.size()) {
                return BLE_ATT_ERR_WRITE_NOT_PERMITTED;
            }

            std::uint16_t bytes_written{};
            auto rc = gatt_svr_write(ctxt->om, self.min_packet_size_,
                                     self.write_memory_.size(),
                                     self.write_memory_.data(), &bytes_written);

            if (rc != 0) {
                ESP_LOGI(name, "Cannot write name to memory\n");
                return rc;
            }

            self.on_write(self.write_memory_.subspan(0, bytes_written));

            ESP_LOGI(name, "Notification/Indication scheduled for "
                           "all subscribed peers.\n");
            ble_gatts_chr_updated(attr_handle);
            return rc;
        }
        return error_handler();

    /* Unknown event */
    default:
        return error_handler();
    }

    return 0;
}

void BLECharacteristicBase::send_update_notification() const {
    ble_gatts_chr_updated(characteristic_value_handle_);
}

void BLEServiceRegistrator::add_service(const ble_gatt_svc_def& service) {
    if (registration_done_) {
        ESP_LOGI("BLEServiceRegistrator",
                 "Cannot add a new service after registration has been done");
        return;
    }

    services_.push_back(service);
}

void BLEServiceRegistrator::register_all_services() {
    if (registration_done_) {
        ESP_LOGI("BLEServiceRegistrator",
                 "services already registered, doing nothing");
        return;
    }

    services_.push_back(ble_gatt_svc_def{.type = 0});

    /* 1. GATT service initialization */
    ble_svc_gatt_init();

    /* 2. Update GATT services counter */
    int rc = ble_gatts_count_cfg(services_.data());
    if (rc != 0) {
        ESP_LOGI("BLEServiceRegistrator", "ble_gatts_count_cfg() failed, rc=%d",
                 rc);
        return;
    }

    /* 3. Add GATT services */
    rc = ble_gatts_add_svcs(services_.data());
    if (rc != 0) {
        ESP_LOGI("BLEServiceRegistrator", "ble_gatts_add_svcs() failed, rc=%d",
                 rc);
        return;
    }

    registration_done_ = true;
    ESP_LOGI("BLEServiceRegistrator", "all %d services registered",
             services_.size() - 1);
}

} // namespace plant
