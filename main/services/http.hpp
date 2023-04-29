#pragma once

#include <esp_err.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

namespace plant {

[[nodiscard]] esp_err_t start_http_service(QueueHandle_t watering_schedule);

} // namespace plant