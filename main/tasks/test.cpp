#include <tasks/test.hpp>
#include <data/watering_schedule.hpp>
#include <hardware/pump.hpp>
#include <hardware/valve.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h>

#include <sys/time.h>

#include <algorithm>

namespace plant {

void test_task(void* arg);

TaskHandle_t create_test_task(TestParams params) {
    static TestParams saved_params = params;
    TaskHandle_t test_handle;
    xTaskCreate(&test_task, "test", configMINIMAL_STACK_SIZE * 4, &saved_params,
                tskIDLE_PRIORITY + 1, &test_handle);
    return test_handle;
}

void test_task(void* arg) {
    auto* params = reinterpret_cast<TestParams*>(arg);
    HardwareState test_config;

    while (true) {
        ESP_LOGI("test_task", "Waiting test request");
        vTaskSuspend(nullptr);
        while (xQueuePeek(params->test_queue, &test_config, 0) == pdTRUE) {
            xQueueSendToBack(params->hardware_queue, &test_config,
                             portMAX_DELAY);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}

} // namespace plant