#pragma once

#include <esp_err.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

namespace plant {

struct HttpServiceParams {
    QueueHandle_t mode_switch_queue;
    QueueHandle_t watering_schedule_queue;
    QueueHandle_t test_configuration_queue;
    QueueHandle_t program_test_queue;
};

[[nodiscard]] esp_err_t start_http_service(HttpServiceParams params);

} // namespace plant