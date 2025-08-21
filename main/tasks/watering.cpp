#include <tasks/watering.hpp>
#include <tasks/hardware.hpp>

#include <data/watering_request.hpp>
#include <data/hardware_state.hpp>
#include <services/ble_watering_service/ble_watering_state.hpp>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <sys/select.h>
#include <sys/time.h>

#include <esp_log.h>

namespace plant {

void watering_task(void* arg);

TaskHandle_t create_watering_task(WateringTaskParams params) {
    static WateringTaskParams saved_params = params;
    TaskHandle_t watering_handle{};
    xTaskCreate(&watering_task, "watering", configMINIMAL_STACK_SIZE * 4,
                &saved_params, tskIDLE_PRIORITY + 2, &watering_handle);
    return watering_handle;
}

void watering_task(void* arg) {
    auto* params = reinterpret_cast<WateringTaskParams*>(arg);

    WateringRequest request;
    HardwareState hw_state;
    WateringState watering_state;

    auto now_ms = [] {
        timeval tv;
        gettimeofday(&tv, nullptr);
        return tv.tv_sec * 1000 + tv.tv_usec / 1000;
    };

    auto stop_watering = [params] {
        const auto state = HardwareState{};
        xQueueSendToBack(params->hardware_queue, &state, 0);
    };

    constexpr auto safe_watchdog_timeout = hardware_task_timeout_ms / 2;

    while (true) {
        if (xQueueReceive(params->watering_queue, &request,
                          safe_watchdog_timeout / portTICK_PERIOD_MS) ==
            pdTRUE) {

            // Clear any pending notifications (stop request when not watering)
            ulTaskNotifyTake(pdTRUE, 0);

            hw_state = HardwareState{};
            hw_state.output_state[request.output] = true;
            hw_state.flow_speed = request.flow_speed;

            // First blocking send to make sure the task is ready to take requests
            xQueueSendToBack(params->hardware_queue, &hw_state, portMAX_DELAY);

            // Compute watering start and end time (millisecond precision)
            const auto start_ms = now_ms();
            const auto end_ms =
                start_ms + static_cast<time_t>(request.duration * 1000);

            watering_state.start_time = start_ms / 1000;
            watering_state.watering_duration = request.duration;
            watering_state.watering = true;
            params->watering_state_characteristic->set_state(watering_state);

            while (true) {
                const auto now = now_ms();
                if (now >= end_ms) {
                    break;
                }

                // Send frequent updates to avoid triggering the HW task watchdog
                xQueueSendToBack(params->hardware_queue, &hw_state, 0);

                // Pick the best wait time between the remaining watering time
                // and HW task watchdog
                const auto time_to_wait =
                    std::min<time_t>(safe_watchdog_timeout, end_ms - now);

                if (ulTaskNotifyTake(
                        pdTRUE, time_to_wait / portTICK_PERIOD_MS) == pdTRUE) {
                    // A stop request was emitted
                    break;
                }
            }

            watering_state.watering = false;
            params->watering_state_characteristic->set_state(watering_state);

            stop_watering();

            if (request.notify_on_completion != nullptr) {
                xTaskNotify(request.notify_on_completion, request.output,
                            eSetValueWithOverwrite);
            }
        } else {
            stop_watering();
        }
    }
}

} // namespace plant