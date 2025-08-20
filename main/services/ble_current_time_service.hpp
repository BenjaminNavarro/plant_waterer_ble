#pragma once

#include <services/ble_service.hpp>

#include <host/ble_gatt.h>

#include <esp_err.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <cstdint>

#include <sys/time.h>

namespace plant {

class BLECurrentTimeCharacteristic
    : public BLECharacteristic<sizeof(std::int64_t)> {
public:
    BLECurrentTimeCharacteristic();

private:
    [[nodiscard]] std::span<const std::byte>
    on_read_access(std::span<std::byte> memory) override final;

    void on_write(std::span<const std::byte> memory) override final;
};

class BLECurrentTimeService
    : public BLEServiceWrapper<BLECurrentTimeCharacteristic> {
public:
    BLECurrentTimeService();
};

} // namespace plant