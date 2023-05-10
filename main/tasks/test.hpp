#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

namespace plant {

struct TestParams {
    QueueHandle_t test_queue;
    QueueHandle_t hardware_queue;
};

[[nodiscard]] TaskHandle_t create_test_task(TestParams params);

} // namespace plant