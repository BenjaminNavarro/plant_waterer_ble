#pragma once

#include <services/ble_service.hpp>

#include <host/ble_gatt.h>

#include <esp_err.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <cstdint>

namespace plant {

// Make sure the data is packed
struct WateringSchedule {
    std::uint64_t start_time{};        // UNIX time
    std::uint32_t watering_period{};   // in seconds
    std::uint16_t watering_duration{}; // in seconds
    std::uint8_t flow_speed{};         // 0-100
    bool enabled{};

    static const auto size = sizeof(start_time) + sizeof(watering_period) +
                             sizeof(watering_duration) + sizeof(flow_speed) +
                             sizeof(enabled);

    void read_from_storage();
    void write_to_storage() const;
};

// Make sure the data is packed
struct WateringTest {
    std::uint16_t duration{10};  // in seconds
    std::uint8_t flow_speed{30}; // 0-100

    static const auto size = sizeof(duration) + sizeof(flow_speed);
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
    friend class BLEServiceRegistrator;

    void service_added() final override;

    static int watering_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                                   struct ble_gatt_access_ctxt* ctxt, void* arg);

    WateringSchedule watering_schedule_;
    WateringTest watering_test_;
};

} // namespace plant