#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

namespace plant {

struct WateringTestParams {
    QueueHandle_t watering_test_queue;
    QueueHandle_t hardware_queue;
};

[[nodiscard]] TaskHandle_t create_watering_test_task(WateringTestParams params);

} // namespace plant