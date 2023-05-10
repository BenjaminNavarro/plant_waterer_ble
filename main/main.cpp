/* HTTP Restful API Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <tasks/blink.hpp>
#include <tasks/watering.hpp>
#include <tasks/watering_test.hpp>
#include <tasks/test.hpp>
#include <tasks/hardware.hpp>

#include <services/ntp.hpp>
#include <services/mdns.hpp>
#include <services/http.hpp>

#include <data/watering_schedule.hpp>

#include <driver/gpio.h>
#include <esp_event.h>
#include <esp_netif.h>
#include <esp_netif_sntp.h>
#include <esp_sntp.h>
#include <esp_log.h>
#include <lwip/apps/netbiosns.h>
#include <mdns.h>
#include <nvs_flash.h>
#include <protocol_examples_common.h>
#include <sdkconfig.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <array>
#include <ctime>
#include <sys/time.h>

esp_err_t start_rest_server();

extern "C" void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    netbiosns_init();
    netbiosns_set_name(CONFIG_EXAMPLE_MDNS_HOST_NAME);

    ESP_ERROR_CHECK(example_connect());
    ESP_ERROR_CHECK(plant::start_mdns_service());

    QueueHandle_t watering_schedule_queue =
        xQueueCreate(1, sizeof(plant::WateringSchedule));
    {
        if (auto saved_schedule = plant::read_schedule_from_storage()) {
            ESP_LOGI("main", "Schedule read from storage");
            xQueueSendToBack(watering_schedule_queue, &saved_schedule.value(),
                             0);
        } else {
            ESP_LOGI("main",
                     "No schedule saved in storage, creating a default one");
            plant::WateringSchedule default_schedule;
            xQueueSendToBack(watering_schedule_queue, &default_schedule, 0);
        }
    }

    QueueHandle_t hardware_queue =
        xQueueCreate(1, sizeof(plant::HardwareState));

    QueueHandle_t test_configuration_queue =
        xQueueCreate(1, sizeof(plant::HardwareState));

    QueueHandle_t watering_test_queue =
        xQueueCreate(1, sizeof(plant::WateringTest));

    plant::create_hardware_task(hardware_queue);

    auto* watering_task = plant::create_watering_task(
        {.watering_schedule_queue = watering_schedule_queue,
         .hardware_queue = hardware_queue});

    auto* test_task =
        plant::create_test_task({.test_queue = test_configuration_queue,
                                 .hardware_queue = hardware_queue});

    auto* watering_test_task = plant::create_watering_test_task(
        {.watering_test_queue = watering_test_queue,
         .hardware_queue = hardware_queue});

    ESP_ERROR_CHECK(plant::start_ntp_service(watering_task));
    ESP_ERROR_CHECK(plant::start_http_service(
        {.watering_task = watering_task,
         .test_task = test_task,
         .watering_test_task = watering_test_task,
         .watering_schedule_queue = watering_schedule_queue,
         .test_configuration_queue = test_configuration_queue,
         .watering_test_queue = watering_test_queue}));

    plant::create_blink_task();
}
