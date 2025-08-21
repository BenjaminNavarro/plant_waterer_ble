#include <tasks/hardware.hpp>
#include <tasks/watering.hpp>

#include <data/hardware_state.hpp>
#include <hardware/pump.hpp>
#include <hardware/valve.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h>

#include <sys/time.h>

#include <string>

namespace plant {

void hardware_task(void* arg);

TaskHandle_t create_hardware_task(QueueHandle_t hardware_queue) {
    TaskHandle_t hardware_handle{};
    xTaskCreate(&hardware_task, "hardware", configMINIMAL_STACK_SIZE * 4,
                hardware_queue, tskIDLE_PRIORITY + 3, &hardware_handle);
    return hardware_handle;
}

void hardware_task(void* arg) {
    // std::array<Valve, 8> valves{
    //     Valve{GPIO_NUM_23}, Valve{GPIO_NUM_4},  Valve{GPIO_NUM_18},
    //     Valve{GPIO_NUM_19}, Valve{GPIO_NUM_13}, Valve{GPIO_NUM_12},
    //     Valve{GPIO_NUM_14}, Valve{GPIO_NUM_27},
    // };
    std::array<Valve, valve_count> valves{Valve{GPIO_NUM_8}};
    WaterPump water_pump{GPIO_NUM_0};

    auto* hardware_queue = reinterpret_cast<QueueHandle_t>(arg);
    HardwareState state;
    HardwareState prev_state;
    std::string message;

    time_t watering_start{};

    auto now_ms = [] {
        timeval tv;
        gettimeofday(&tv, nullptr);
        return tv.tv_sec * 1000 + tv.tv_usec / 1000;
    };

    while (true) {
        if (xQueueReceive(hardware_queue, &state,
                          hardware_task_timeout_ms / portTICK_PERIOD_MS) != 0) {
            if (state != prev_state) {
                prev_state = state;

                message.clear();
                message = "Request received (pump: ";
                message += std::to_string(state.flow_speed);
                message += ", outputs: [";

                // Can't seem to format a range with current version of std lib...
                for (std::size_t i = 0; i < state.output_state.size(); i++) {
                    const bool is_last = i == state.output_state.size() - 1;
                    message += state.output_state[i] ? '1' : '0';
                    if (!is_last) {
                        message += ", ";
                    }
                }
                message += "])\n";

                if (state.flow_speed > 0 and watering_start == 0) {
                    watering_start = now_ms();
                }

                if (state.flow_speed == 0 and watering_start > 0) {
                    auto watering_end = now_ms();
                    ESP_LOGI("hardware_task", "finished watering for %fs",
                             static_cast<float>(watering_end - watering_start) /
                                 1000.f);
                    watering_start = 0;
                }

                ESP_LOGI("hardware_task", "%s", message.c_str());
            }

            for (std::size_t i = 0; i < state.output_state.size(); i++) {
                if (state.output_state[i]) {
                    valves[i].open();
                } else {
                    valves[i].close();
                }
            }

            water_pump.set_flow_speed(
                FlowSpeed{static_cast<float>(state.flow_speed) / 100.f});
        } else {
            ESP_LOGI("hardware_task", "Timeout, stopping everything");
            for (auto& valve : valves) {
                valve.close();
            }
            water_pump.turn_off();
        }
    }
}

} // namespace plant