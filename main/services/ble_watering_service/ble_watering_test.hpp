#pragma once

#include <services/ble_service.hpp>

#include <host/ble_gatt.h>

#include <esp_err.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <cstdint>

namespace plant {

// Make sure the data is packed
struct WateringTest {
    std::uint16_t duration{10};  // in seconds
    std::uint8_t flow_speed{30}; // 0-100

    static const auto size = sizeof(duration) + sizeof(flow_speed);
};

class BLEWateringTestCharacteristic
    : public BLECharacteristic<sizeof(WateringTest)> {
public:
    BLEWateringTestCharacteristic();

    [[nodiscard]] const WateringTest& test() const {
        return test_;
    }

    [[nodiscard]] WateringTest& test() {
        return test_;
    };

private:
    void on_write(std::span<const std::byte> memory) override final;

    WateringTest test_;
};

} // namespace plant