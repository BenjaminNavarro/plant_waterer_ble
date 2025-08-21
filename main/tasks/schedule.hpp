#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

namespace plant {

// TODO 3 tasks:
//   1. Hardware task: same as before, handle IO + watchdog
//   2. Watering task: listen to a queue to watering commands (duration + flow +
//      output) and pilot the HW task.
//      This makes sure that all commands are processed in sequence no matter
//      where they come from
//   3. Schedule task: send watering commands to the watering task when a
//      scheduled watering has to be done

struct ScheduleTaskParams {
    QueueHandle_t watering_schedule_queue; // IN
    QueueHandle_t watering_requests_queue; // OUT
};

TaskHandle_t create_schedule_task(ScheduleTaskParams params);

} // namespace plant