#pragma once

#include <services/ble_service.hpp>

#include <host/ble_gatt.h>

#include <esp_err.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

namespace plant {

struct BLECurrentTime {
    // https://iot.stackexchange.com/a/7887
    uint16_t year{2025};
    uint8_t month{5};
    uint8_t day{30};
    uint8_t hours{19};
    uint8_t minutes{01};
    uint8_t seconds{10};
    uint8_t day_of_week{1};
    uint8_t fractions_256{0};
    uint8_t adjust_reason{0x03}; // manual update
};

class BLECurrentTimeService : public BLEService<1> {
public:
    BLECurrentTimeService();

    [[nodiscard]] const BLECurrentTime& current_time() const {
        return current_time_;
    }

    [[nodiscard]] BLECurrentTime& current_time() {
        return current_time_;
    };

private:
    static int current_time_chr_access(uint16_t conn_handle,
                                       uint16_t attr_handle,
                                       struct ble_gatt_access_ctxt* ctxt,
                                       void* arg);

    BLECurrentTime current_time_;
};

} // namespace plant