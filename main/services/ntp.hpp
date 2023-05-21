#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <esp_err.h>

namespace plant {

[[nodiscard]] esp_err_t start_ntp_service(QueueHandle_t mode_switch_queue);

} // namespace plant