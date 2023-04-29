#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_err.h>

namespace plant {

[[nodiscard]] esp_err_t start_ntp_service(TaskHandle_t to_notify);

} // namespace plant