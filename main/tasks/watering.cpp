#include <tasks/watering.hpp>
#include <data/watering_schedule.hpp>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h>

#include <sys/time.h>

#include <algorithm>
#include <limits>

namespace plant {

void watering_task(void* arg);

TaskHandle_t create_watering_task(WateringTaskParams params) {
    static WateringTaskParams saved_params = params;
    TaskHandle_t watring_handle;
    xTaskCreate(&watering_task, "watering", configMINIMAL_STACK_SIZE * 4,
                &saved_params, tskIDLE_PRIORITY + 1, &watring_handle);
    return watring_handle;
}

class AutomaticWatering {
public:
    struct Params {
        QueueHandle_t watering_schedule_queue;
        QueueHandle_t hardware_queue;
    };

    explicit AutomaticWatering(Params params) : params_{params} {
    }

    void execute() {
        xQueuePeek(params_.watering_schedule_queue, &watering_schedule_,
                   portMAX_DELAY);

        struct timeval now_tv;
        gettimeofday(&now_tv, nullptr);
        const auto now = now_tv.tv_sec;

        HardwareState state{};

        // Find the first valve that should be opened
        // We cannot have multiple valves opened at the same time because they
        // might require different flow speeds
        auto* selected_output = watering_schedule_.end();
        for (std::size_t i = 0; i < watering_schedule_.size(); i++) {
            const auto& group_schedule = watering_schedule_[i];
            auto& valve = state.output_state[i];

            if (not group_schedule.enabled) {
                continue;
            }

            const auto watering_start =
                compute_watering_start(group_schedule, now);
            const auto watering_end =
                watering_start + group_schedule.watering_duration;

            // Another valve could have delayed the openning of this one so we
            // have to measure this delay to compensate it
            if (now >= watering_start and
                last_watering_start_[i] != watering_start) {
                last_watering_start_[i] = watering_start;
                start_delay_[i] = now - watering_start;
            }

            if (now >= (watering_start + start_delay_[i]) and
                now < (watering_end + start_delay_[i])) {
                valve = true;
                selected_output = watering_schedule_.begin() + i;
                break;
            }
        }

        const auto water_should_flow =
            selected_output != watering_schedule_.end();

        if (water_should_flow) {
            state.flow_speed = selected_output->flow_speed;
        } else {
            state.flow_speed = FlowSpeed{};
        }

        xQueueSendToBack(params_.hardware_queue, &state, portMAX_DELAY);
    }

private:
    static std::uint64_t
    compute_watering_start(const PlantGroupSchedule& schedule,
                           std::uint64_t now) {
        if (schedule.watering_period == 0) {
            return std::numeric_limits<std::uint64_t>::max();
        } else if (schedule.start_time >= now) {
            // start_time in the future
            return schedule.start_time;
        } else {
            // start_time in the past
            const auto cycles =
                (now - schedule.start_time) / schedule.watering_period;
            return schedule.start_time + cycles * schedule.watering_period;
        }
    };

    Params params_;
    WateringSchedule watering_schedule_;
    std::array<std::uint64_t, valve_count> start_delay_{};
    std::array<std::uint64_t, valve_count> last_watering_start_{};
};

class ManualControl {
public:
    struct Params {
        QueueHandle_t test_configuration_queue;
        QueueHandle_t hardware_queue;
    };

    explicit ManualControl(Params params) : params_{params} {
    }

    void reset() {
        test_config_ = {};
    }

    void execute() {
        xQueueReceive(params_.test_configuration_queue, &test_config_, 0);
        xQueueSendToBack(params_.hardware_queue, &test_config_, portMAX_DELAY);
    }

private:
    Params params_;
    HardwareState test_config_;
};

class ProgramTester {
public:
    struct Params {
        QueueHandle_t program_test_queue;
        QueueHandle_t hardware_queue;
    };

    explicit ProgramTester(Params params) : params_{params} {
    }

    bool execute() {
        if (xQueueReceive(params_.program_test_queue, &test_config_, 0) != 0) {
            ESP_LOGI(
                "program_test_task",
                "Watering test request received (output=%zu, duration=%llus)",
                test_config_.output, test_config_.duration);

            end_ = xTaskGetTickCount() +
                   (test_config_.duration * 1000) / portTICK_PERIOD_MS;
        }

        HardwareState state;
        const bool is_still_running = xTaskGetTickCount() < end_;

        if (is_still_running) {
            state.output_state[test_config_.output] = true;
            state.flow_speed = test_config_.flow_speed;
        }

        xQueueSendToBack(params_.hardware_queue, &state, portMAX_DELAY);

        return is_still_running;
    }

private:
    Params params_;
    WateringTest test_config_;
    std::optional<std::uint64_t> end_;
};

void watering_task(void* arg) {
    auto* params = reinterpret_cast<WateringTaskParams*>(arg);

    AutomaticWatering automatic_watering{
        {.watering_schedule_queue = params->watering_schedule_queue,
         .hardware_queue = params->hardware_queue}};

    ManualControl manual_control{
        {.test_configuration_queue = params->test_configuration_queue,
         .hardware_queue = params->hardware_queue}};

    ProgramTester program_tester{
        {.program_test_queue = params->program_test_queue,
         .hardware_queue = params->hardware_queue}};

    const HardwareState all_off;

    Mode prev_mode{Mode::Idle};
    Mode mode{Mode::Idle};

    TickType_t last_wake_time = xTaskGetTickCount();
    const TickType_t task_frequency = 100 / portTICK_PERIOD_MS;

    while (true) {
        xQueueReceive(params->mode_switch_queue, &mode, 0);

        const bool mode_has_changed = mode != prev_mode;

        if (mode_has_changed) {
            ESP_LOGI("watering_task", "Switching to %s mode",
                     to_string(mode).data());
            xQueueSend(params->hardware_queue, &all_off, portMAX_DELAY);
        }

        switch (mode) {
        case Mode::Automatic: {
            automatic_watering.execute();
        } break;
        case Mode::Manual: {
            if (mode_has_changed) {
                manual_control.reset();
            }

            manual_control.execute();
        } break;
        case Mode::WateringTest: {
            const bool test_is_running = program_tester.execute();

            if (not test_is_running) {
                constexpr auto new_mode{Mode::Automatic};
                xQueueSend(params->mode_switch_queue, &new_mode, portMAX_DELAY);
            }
        } break;
        case Mode::Idle: {
            xQueueSend(params->hardware_queue, &all_off, portMAX_DELAY);
        } break;
        }

        prev_mode = mode;

        vTaskDelayUntil(&last_wake_time, task_frequency);
    }
}

} // namespace plant