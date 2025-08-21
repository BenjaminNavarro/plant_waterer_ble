#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <cstdint>

namespace plant {

struct WateringRequest {
    std::uint16_t duration{};  // in seconds
    std::uint8_t flow_speed{}; // 0-100
    std::uint8_t output{};
    TaskHandle_t notify_on_completion{};
};

} // namespace plant