#include <tasks/watering.hpp>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h>

namespace plant {

void watering_task(void* /*unused*/);

TaskHandle_t create_watering_task() {
    TaskHandle_t watering_handle;
    xTaskCreate(&watering_task, "watering", configMINIMAL_STACK_SIZE * 4,
                nullptr, tskIDLE_PRIORITY + 2, &watering_handle);
    return watering_handle;
}

void watering_task(void* /*unused*/) {
    ESP_LOGI("watering_task", "Waiting notification to start");
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    ESP_LOGI("watering_task", "Start notification received");

    while (true) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

} // namespace plant