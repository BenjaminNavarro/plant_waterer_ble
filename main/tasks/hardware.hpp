#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

namespace plant {

constexpr TickType_t hardware_task_timeout_ms = 1000;

TaskHandle_t create_hardware_task(QueueHandle_t hardware_queue);

} // namespace plant