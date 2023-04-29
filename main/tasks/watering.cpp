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

TaskHandle_t create_watering_task(QueueHandle_t watering_schedule) {
    TaskHandle_t watering_handle;
    xTaskCreate(&watering_task, "watering", configMINIMAL_STACK_SIZE * 4,
                watering_schedule, tskIDLE_PRIORITY + 1, &watering_handle);
    return watering_handle;
}

void watering_task(void* arg) {
    auto* watering_schedule_queue = reinterpret_cast<QueueHandle_t>(arg);
    WateringSchedule watering_schedule;
    std::array<Valve, watering_schedule.size()> valves{
        Valve{GPIO_NUM_23}, Valve{GPIO_NUM_4},  Valve{GPIO_NUM_16},
        Valve{GPIO_NUM_17}, Valve{GPIO_NUM_13}, Valve{GPIO_NUM_12},
        Valve{GPIO_NUM_14}, Valve{GPIO_NUM_27},
    };
    WaterPump water_pump{GPIO_NUM_15};

    ESP_LOGI("watering_task", "Waiting notification to start");
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    ESP_LOGI("watering_task", "Start notification received");

    auto compute_watering_start = [](const PlantGroupSchedule& schedule,
                                     std::uint64_t now) {
        const auto cycles =
            (now - schedule.start_time) / schedule.watering_period;
        return schedule.start_time + cycles * schedule.watering_period;
    };

    TickType_t last_wake_time = xTaskGetTickCount();
    const TickType_t task_frequency = 1000 / portTICK_PERIOD_MS;

    while (true) {
        xQueuePeek(watering_schedule_queue, watering_schedule.data(),
                   portMAX_DELAY);

        struct timeval now_tv;
        gettimeofday(&now_tv, nullptr);
        const auto now = now_tv.tv_sec;

        for (std::size_t i = 0; i < watering_schedule.size(); i++) {
            const auto& group_schedule = watering_schedule[i];
            auto& valve = valves[i];

            if (not group_schedule.enabled) {
                valve.close();
                continue;
            }

            const auto watering_start =
                compute_watering_start(group_schedule, now);
            const auto watering_end =
                watering_start + group_schedule.watering_duration;

            if (now >= watering_start and now < watering_end) {
                if (not valve.is_open()) {
                    ESP_LOGI("watering_task", "Opening valve %d", (int)i + 1);
                }
                valve.open();
            } else {
                if (valve.is_open()) {
                    ESP_LOGI("watering_task", "Closing valve %d", (int)i + 1);
                }
                valve.close();
            }
        }

        const auto water_should_flow =
            std::any_of(begin(valves), end(valves),
                        [](const Valve& valve) { return valve.is_open(); });

        if (water_should_flow) {
            if (not water_pump.is_enabled()) {
                ESP_LOGI("watering_task", "Turning on the water pump");
            }
            water_pump.enable();
        } else {
            if (water_pump.is_enabled()) {
                ESP_LOGI("watering_task", "Turning off the water pump");
            }
            water_pump.disable();
        }

        vTaskDelayUntil(&last_wake_time, task_frequency);
    }
}

} // namespace plant