#include <tasks/watering.hpp>
#include <data/watering_schedule.hpp>
#include <hardware/pump.hpp>
#include <hardware/valve.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h>

#include <sys/time.h>

#include <algorithm>

namespace plant {

void watering_task(void* arg);

TaskHandle_t create_watering_task(WateringTaskParams params) {
    static WateringTaskParams saved_params = params;
    TaskHandle_t watering_handle;
    xTaskCreate(&watering_task, "watering", configMINIMAL_STACK_SIZE * 4,
                &saved_params, tskIDLE_PRIORITY + 1, &watering_handle);
    return watering_handle;
}

void watering_task(void* arg) {

    ESP_LOGI("watering_task", "Waiting notification to start");
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    ESP_LOGI("watering_task", "Start notification received");

    auto compute_watering_start = [](const PlantGroupSchedule& schedule,
                                     std::uint64_t now) {
        const auto cycles =
            (now - schedule.start_time) / schedule.watering_period;
        return schedule.start_time + cycles * schedule.watering_period;
    };

    auto* params = reinterpret_cast<WateringTaskParams*>(arg);
    WateringSchedule watering_schedule;
    HardwareState state;

    TickType_t last_wake_time = xTaskGetTickCount();
    const TickType_t task_frequency = 1000 / portTICK_PERIOD_MS;

    while (true) {
        xQueuePeek(params->watering_schedule_queue, &watering_schedule,
                   portMAX_DELAY);

        struct timeval now_tv;
        gettimeofday(&now_tv, nullptr);
        const auto now = now_tv.tv_sec;

        for (std::size_t i = 0; i < watering_schedule.size(); i++) {
            const auto& group_schedule = watering_schedule[i];
            auto& valve = state.output_state[i];

            if (not group_schedule.enabled) {
                valve = false;
                continue;
            }

            const auto watering_start =
                compute_watering_start(group_schedule, now);
            const auto watering_end =
                watering_start + group_schedule.watering_duration;

            if (now >= watering_start and now < watering_end) {
                if (not valve) {
                    ESP_LOGI("watering_task", "Opening valve %d", (int)i + 1);
                }
                valve = true;
            } else {
                if (valve) {
                    ESP_LOGI("watering_task", "Closing valve %d", (int)i + 1);
                }
                valve = false;
            }
        }

        const auto water_should_flow =
            std::any_of(begin(state.output_state), end(state.output_state),
                        [](bool value) { return value; });

        if (water_should_flow) {
            if (not state.pump_state) {
                ESP_LOGI("watering_task", "Turning on the water pump");
            }
            state.pump_state = true;
        } else {
            if (state.pump_state) {
                ESP_LOGI("watering_task", "Turning off the water pump");
            }
            state.pump_state = false;
        }

        xQueueSendToBack(params->hardware_queue, &state, portMAX_DELAY);

        vTaskDelayUntil(&last_wake_time, task_frequency);
    }
}

} // namespace plant