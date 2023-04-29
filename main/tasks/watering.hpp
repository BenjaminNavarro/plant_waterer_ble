#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace plant {

[[nodiscard]] TaskHandle_t create_watering_task();

} // namespace plant