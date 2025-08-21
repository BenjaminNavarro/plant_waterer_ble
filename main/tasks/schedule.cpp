#include <tasks/schedule.hpp>
#include <data/watering_program.hpp>
#include <data/watering_request.hpp>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h>

#include <sys/time.h>

#include <limits>

namespace plant {

void schedule_task(void* arg);

TaskHandle_t create_schedule_task(ScheduleTaskParams params) {
    static ScheduleTaskParams saved_params = params;
    TaskHandle_t schedule_handle;
    xTaskCreate(&schedule_task, "schedule", configMINIMAL_STACK_SIZE * 4,
                &saved_params, tskIDLE_PRIORITY + 1, &schedule_handle);
    return schedule_handle;
}

class AutomaticSchedule {
public:
    static constexpr auto infinite_time =
        std::numeric_limits<std::uint64_t>::max();

    explicit AutomaticSchedule(ScheduleTaskParams params) : params_{params} {
    }

    void execute() {
        timeval now_tv;
        gettimeofday(&now_tv, nullptr);
        const auto now = now_tv.tv_sec;

        check_for_new_program(now);
        check_for_watering_completion(now);

        for (std::size_t i = 0; i < valve_count; i++) {
            auto& data = program_data_[i];
            const auto& program = programs_[i];
            if (now >= data.start_time and not data.started) {
                data.started = true;

                WateringRequest request;
                request.output = i;
                request.duration = program.watering_duration;
                request.flow_speed = program.flow_speed;
                request.notify_on_completion = xTaskGetCurrentTaskHandle();

                ESP_LOGI(
                    "schedule",
                    "sending watering request for output %u (%us @ %u%%)\n",
                    request.output, request.duration, request.flow_speed);

                xQueueSendToBack(params_.watering_requests_queue, &request,
                                 portMAX_DELAY);
            }
        }
    }

private:
    void check_for_new_program(std::uint64_t now) {
        WateringProgram program;
        if (xQueueReceive(params_.watering_schedule_queue, &program, 0) ==
            pdTRUE) {
            programs_[program.output] = program.schedule;

            auto& data = program_data_[program.output];

            data = ProgramData{};
            data.start_time = compute_schedule_start(program.schedule, now);

            ESP_LOGI("schedule",
                     "new program received for output %u (now = %llu, start = "
                     "%llu, next = %llu)\n",
                     program.output, now, program.schedule.start_time,
                     data.start_time);
        }
    }

    void check_for_watering_completion(std::uint64_t now) {
        std::uint32_t output{};
        if (xTaskNotifyWait(0, 0, &output, 0) == pdTRUE) {
            auto& data = program_data_[output];

            data = ProgramData{};
            data.start_time = compute_schedule_start(programs_[output], now);

            ESP_LOGI("schedule",
                     "watering completion notification received for output %lu "
                     "(now = %llu, start = %llu, next = %llu)\n",
                     output, now, programs_[output].start_time, data.start_time);
        }
    }

    static std::uint64_t compute_schedule_start(const WateringSchedule& schedule,
                                                std::uint64_t now) {
        if (schedule.watering_period == 0 or not schedule.enabled) {
            return infinite_time;
        } else if (now <= schedule.start_time) {
            // start_time in the future
            return schedule.start_time;
        } else {
            // start_time in the past
            const auto delta = now - schedule.start_time;
            auto cycles = delta / schedule.watering_period;
            if (delta % schedule.watering_period != 0) {
                ++cycles;
            }
            return schedule.start_time + cycles * schedule.watering_period;
        }
    };

    struct ProgramData {
        std::uint64_t start_time{infinite_time};
        bool started{};
    };

    ScheduleTaskParams params_;
    std::array<ProgramData, valve_count> program_data_;
    std::array<WateringSchedule, valve_count> programs_;
};

void schedule_task(void* arg) {
    auto* params = reinterpret_cast<ScheduleTaskParams*>(arg);

    AutomaticSchedule automatic_schedule{*params};

    TickType_t last_wake_time = xTaskGetTickCount();
    const TickType_t task_frequency = 100 / portTICK_PERIOD_MS;

    while (true) {
        automatic_schedule.execute();

        vTaskDelayUntil(&last_wake_time, task_frequency);
    }
}

} // namespace plant