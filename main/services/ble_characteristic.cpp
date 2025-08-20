#include <services/ble_characteristic.hpp>

#include <esp_log.h>
#include <host/ble_hs.h>
#include <services/gatt/ble_svc_gatt.h>

namespace plant {

namespace {

int gatt_svr_write(struct os_mbuf* om, uint16_t min_len, uint16_t max_len,
                   void* dst, uint16_t* len) {
    uint16_t om_len;
    int rc;

    om_len = OS_MBUF_PKTLEN(om);
    if (om_len < min_len || om_len > max_len) {
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }

    rc = ble_hs_mbuf_to_flat(om, dst, max_len, len);
    if (rc != 0) {
        return BLE_ATT_ERR_UNLIKELY;
    }

    return 0;
}

} // namespace

BLECharacteristicBase::BLECharacteristicBase(std::string_view name,
                                             const ble_uuid_t* uuid,
                                             ble_gatt_chr_flags flags,
                                             std::span<std::byte> read_memory,
                                             std::span<std::byte> write_memory,
                                             std::uint16_t min_packet_size)
    : name_{name},
      read_memory_{read_memory},
      write_memory_{write_memory},
      min_packet_size_{min_packet_size} {
    characteristic_settings_.uuid = uuid;
    characteristic_settings_.flags = flags;
    characteristic_settings_.access_cb = on_access;
    characteristic_settings_.arg = this;
    characteristic_settings_.val_handle = &characteristic_value_handle_;
}

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
                ESP_LOGI(name, "Cannot write data to memory\n");
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

} // namespace plant
