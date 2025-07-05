#pragma once

#include <services/ble_service.hpp>

#include <host/ble_gatt.h>

#include <esp_err.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

namespace plant {

class BLEBatteryService : public BLEService<1> {
public:
    BLEBatteryService();

    [[nodiscard]] const float& battery_level() const {
        return battery_level_;
    }

    [[nodiscard]] float& battery_level() {
        return battery_level_;
    };

private:
    static int battery_level_chr_access(uint16_t conn_handle,
                                        uint16_t attr_handle,
                                        struct ble_gatt_access_ctxt* ctxt,
                                        void* arg);

    float battery_level_;
};

} // namespace plant