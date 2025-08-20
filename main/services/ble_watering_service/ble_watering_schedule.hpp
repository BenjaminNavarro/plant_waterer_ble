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

class BLEWateringScheduleCharacteristic
    : public BLECharacteristic<WateringSchedule::size> {
public:
    BLEWateringScheduleCharacteristic();

    [[nodiscard]] const WateringSchedule& schedule() const {
        return schedule_;
    }

    [[nodiscard]] WateringSchedule& schedule() {
        return schedule_;
    };

private:
    [[nodiscard]] std::span<const std::byte>
    on_read_access(std::span<std::byte> memory) override final;

    void on_write(std::span<const std::byte> memory) override final;

    WateringSchedule schedule_;
};

} // namespace plant