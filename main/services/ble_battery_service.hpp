#pragma once

#include <services/ble_service.hpp>

#include <host/ble_gatt.h>

#include <esp_err.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

namespace plant {

class BLEBatteryCharacteristic
    : public BLECharacteristic<sizeof(std::uint8_t)> {
public:
    BLEBatteryCharacteristic();

    [[nodiscard]] const float& battery_level() const {
        return battery_level_;
    }

    [[nodiscard]] float& battery_level() {
        return battery_level_;
    };

private:
    [[nodiscard]] std::span<const std::byte>
    on_read_access(std::span<std::byte> memory) override final;

    float battery_level_;
};

class BLEBatteryService : public BLEServiceWrapper<BLEBatteryCharacteristic> {
public:
    BLEBatteryService();

    [[nodiscard]] const float& battery_level() const {
        return get_characteristic<0>().battery_level();
    }

    [[nodiscard]] float& battery_level() {
        return get_characteristic<0>().battery_level();
    };
};

} // namespace plant