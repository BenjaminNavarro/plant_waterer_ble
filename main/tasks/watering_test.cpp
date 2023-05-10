#include <tasks/watering_test.hpp>
#include <data/watering_schedule.hpp>
#include <hardware/pump.hpp>
#include <hardware/valve.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h>

#include <sys/time.h>

#include <algorithm>

namespace plant {

void watering_test_task(void* arg);

TaskHandle_t create_watering_test_task(WateringTestParams params) {
    static WateringTestParams saved_params = params;
    TaskHandle_t watering_test_handle;
    xTaskCreate(&watering_test_task, "watering_test",
                configMINIMAL_STACK_SIZE * 4, &saved_params,
                tskIDLE_PRIORITY + 1, &watering_test_handle);
    return watering_test_handle;
}

void watering_test_task(void* arg) {
    auto* params = reinterpret_cast<WateringTestParams*>(arg);

    WateringTest test_config;

    const TickType_t task_frequency = 1000 / portTICK_PERIOD_MS;

    while (true) {
        ESP_LOGI("watering_test_task", "Waiting new watering test request");
        xQueueReceive(params->watering_test_queue, &test_config, portMAX_DELAY);
        ESP_LOGI("watering_test_task",
                 "Watering test request received (output=%zu, duration=%llus)",
                 test_config.output, test_config.duration);

        const auto end = xTaskGetTickCount() +
                         (test_config.duration * 1000) / portTICK_PERIOD_MS;

        TickType_t last_wake_time = xTaskGetTickCount();

        HardwareState state;
        state.output_state[test_config.duration] = true;
        state.pump_state = true;

        while (xTaskGetTickCount() < end) {
            xQueueSendToBack(params->hardware_queue, &state, portMAX_DELAY);
            vTaskDelayUntil(&last_wake_time, task_frequency);
        }

        state = HardwareState{};
        xQueueSendToBack(params->hardware_queue, &state, portMAX_DELAY);
    }
}

} // namespace plant