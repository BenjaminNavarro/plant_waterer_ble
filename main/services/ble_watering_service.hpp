#pragma once

#include <services/ble_service.hpp>

#include <host/ble_gatt.h>

#include <esp_err.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <cstdint>

namespace plant {

struct WateringSchedule {
    std::uint64_t start_time{1751472531}; // UNIX time
    std::uint32_t watering_period{86400}; // in seconds
    std::uint16_t watering_duration{30};  // in seconds
    std::uint8_t flow_speed{50};          // 0-100
    bool enabled{false};
};

struct WateringTest {
    std::uint16_t duration{10};  // in seconds
    std::uint8_t flow_speed{30}; // 0-100
};

class BLEWateringService : public BLEService<2> {
public:
    BLEWateringService();

    [[nodiscard]] const WateringSchedule& watering_schedule() const {
        return watering_schedule_;
    }

    [[nodiscard]] WateringSchedule& watering_schedule() {
        return watering_schedule_;
    };

    [[nodiscard]] const WateringTest& watering_test() const {
        return watering_test_;
    }

    [[nodiscard]] WateringTest& watering_test() {
        return watering_test_;
    }

private:
    static int watering_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                                   struct ble_gatt_access_ctxt* ctxt, void* arg);

    WateringSchedule watering_schedule_;
    WateringTest watering_test_;
};

} // namespace plant