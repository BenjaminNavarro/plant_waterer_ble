#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include <pid/unreachable.h>
#include <pid/hashed_string.h>

#include <optional>
#include <string_view>

namespace plant {

enum class Mode { Automatic, Manual, WateringTest, Idle };

constexpr std::string_view to_string(Mode mode) {
    switch (mode) {
    case Mode::Automatic:
        return "automatic";
    case Mode::Manual:
        return "manual";
    case Mode::WateringTest:
        return "program_test";
    case Mode::Idle:
        return "idle";
    }
    pid::unreachable();
}

constexpr std::optional<Mode> mode_from_string(std::string_view mode) {
    using namespace pid::literals;
    switch (pid::hashed_string(mode)) {
    case "automatic"_hs:
        return plant::Mode::Automatic;
    case "manual"_hs:
        return plant::Mode::Manual;
    case "program_test"_hs:
        return plant::Mode::WateringTest;
    case "idle"_hs:
        return plant::Mode::Idle;
    default:
        return std::nullopt;
    }
}

struct WateringTaskParams {
    QueueHandle_t mode_switch_queue;
    QueueHandle_t hardware_queue;
    QueueHandle_t watering_schedule_queue;
    QueueHandle_t test_configuration_queue;
    QueueHandle_t program_test_queue;
};

TaskHandle_t create_watering_task(WateringTaskParams params);

} // namespace plant