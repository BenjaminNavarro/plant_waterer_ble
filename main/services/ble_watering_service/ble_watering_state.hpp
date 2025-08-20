#pragma once

#include <services/ble_service.hpp>

#include <host/ble_gatt.h>

#include <esp_err.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <cstdint>

namespace plant {

// Make sure the data is packed
struct WateringState {
    std::uint64_t start_time{}; // UNIX time
    std::uint16_t watering_duration{};
    bool watering{};

    static const auto size =
        sizeof(start_time) + sizeof(watering_duration) + sizeof(watering);
};

class BLEWateringStateCharacteristic
    : public BLECharacteristic<sizeof(WateringState)> {
public:
    BLEWateringStateCharacteristic();

    [[nodiscard]] const WateringState& state() const {
        return state_;
    }

    void set_state(const WateringState& state) {
        state_ = state;
        notify_watering_state_change();
    };

private:
    [[nodiscard]] std::span<const std::byte>
    on_read_access(std::span<std::byte> memory) override final;

    void notify_watering_state_change();

    WateringState state_;
};

} // namespace plant