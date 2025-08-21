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
    : public BLECharacteristic<WateringTest::size> {
public:
    BLEWateringTestCharacteristic();

    [[nodiscard]] const WateringTest& test() const {
        return test_;
    }

    [[nodiscard]] WateringTest& test() {
        return test_;
    };

    void set_watering_requests_queue(QueueHandle_t watering_requests_queue) {
        watering_requests_queue_ = watering_requests_queue;
    }

    void set_watering_task_handle(TaskHandle_t watering_task) {
        watering_task_ = watering_task;
    }

private:
    void on_write(std::span<const std::byte> memory) override final;

    WateringTest test_;
    QueueHandle_t watering_requests_queue_{};
    TaskHandle_t watering_task_{};
};

} // namespace plant