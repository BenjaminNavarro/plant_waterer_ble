#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

namespace plant {

TaskHandle_t create_hardware_task(QueueHandle_t hardware_queue);

} // namespace plant