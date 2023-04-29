#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

namespace plant {

[[nodiscard]] TaskHandle_t create_watering_task(QueueHandle_t watering_schedule);

} // namespace plant