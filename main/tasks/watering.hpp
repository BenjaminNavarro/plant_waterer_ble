#pragma once

#include <services/ble_watering_service/ble_watering_state.hpp>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

namespace plant {

struct WateringTaskParams {
    QueueHandle_t watering_queue{}; // IN
    QueueHandle_t hardware_queue{}; // OUT
    BLEWateringStateCharacteristic* watering_state_characteristic{};
};

TaskHandle_t create_watering_task(WateringTaskParams params);

} // namespace plant