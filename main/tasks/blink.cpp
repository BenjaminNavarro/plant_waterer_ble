#include <tasks/blink.hpp>

#include <esp_log.h>
#include <driver/gpio.h>
#include <hal/gpio_types.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace plant {

void create_blink_task() {
    TaskHandle_t blink_handle;
    xTaskCreate(
        [](void*) {
            ESP_LOGI("blink", "Task created");
            gpio_reset_pin(GPIO_NUM_2);
            gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
            bool state = false;
            while (true) {
                gpio_set_level(GPIO_NUM_2, state ? 1 : 0);
                state = !state;
                vTaskDelay(500 / portTICK_PERIOD_MS);
            }
        },
        "blink", configMINIMAL_STACK_SIZE * 2, nullptr, tskIDLE_PRIORITY,
        &blink_handle);
}

} // namespace plant