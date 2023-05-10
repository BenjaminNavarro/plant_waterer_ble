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

void hardware_task(void* arg);

TaskHandle_t create_hardware_task(QueueHandle_t hardware_queue) {
    TaskHandle_t hardware_handle;
    xTaskCreate(&hardware_task, "hardware", configMINIMAL_STACK_SIZE * 4,
                hardware_queue, tskIDLE_PRIORITY + 2, &hardware_handle);
    return hardware_handle;
}

void hardware_task(void* arg) {
    std::array<Valve, 8> valves{
        Valve{GPIO_NUM_23}, Valve{GPIO_NUM_4},  Valve{GPIO_NUM_16},
        Valve{GPIO_NUM_17}, Valve{GPIO_NUM_13}, Valve{GPIO_NUM_12},
        Valve{GPIO_NUM_14}, Valve{GPIO_NUM_27},
    };
    WaterPump water_pump{GPIO_NUM_15};

    auto* hardware_queue = reinterpret_cast<QueueHandle_t>(arg);
    HardwareState state;

    constexpr TickType_t receive_timeout = 2000 / portTICK_PERIOD_MS;
    while (true) {
        if (xQueueReceive(hardware_queue, &state, receive_timeout) != 0) {
            ESP_LOGI("hardware_task",
                     "Request received (pump: %d, outputs: "
                     "[%d, %d, %d, %d, %d, %d, %d, %d])",
                     state.pump_state, state.output_state[0],
                     state.output_state[1], state.output_state[2],
                     state.output_state[3], state.output_state[4],
                     state.output_state[5], state.output_state[6],
                     state.output_state[7]);
            for (size_t i = 0; i < state.output_state.size(); i++) {
                if (state.output_state[i]) {
                    valves[i].open();
                } else {
                    valves[i].close();
                }
            }

            if (state.pump_state) {
                water_pump.enable();
            } else {
                water_pump.disable();
            }
        } else {
            ESP_LOGI("hardware_task", "Timeout, stopping everything");
            for (size_t i = 0; i < state.output_state.size(); i++) {
                valves[i].close();
            }
            water_pump.disable();
        }
    }
}

} // namespace plant