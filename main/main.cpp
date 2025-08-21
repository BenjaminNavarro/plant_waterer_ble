/* HTTP Restful API Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

// #include <tasks/blink.hpp>
#include <tasks/hardware.hpp>
#include <tasks/watering.hpp>
#include <tasks/schedule.hpp>

#include <data/hardware_state.hpp>
#include <data/watering_request.hpp>

#include <services/ble_service.hpp>
#include <services/ble_current_time_service.hpp>
#include <services/ble_battery_service.hpp>
#include <services/ble_watering_service.hpp>
#include <services/ble_name_service.hpp>
#include <data/user_defined_name.hpp>
#include <data/watering_program.hpp>

#include "ble/common.h"
#include "ble/gap.hpp"

#include <esp_event.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <sdkconfig.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <array>
#include <ctime>
#include <sys/time.h>

/* Library function declarations */
extern "C" void ble_store_config_init();

/* Private function declarations */
static void on_stack_reset(int reason);
static void on_stack_sync();
static void nimble_host_config_init();
static void nimble_host_task(void* param);

/* Private functions */
/*
 *  Stack event callback functions
 *      - on_stack_reset is called when host resets BLE stack due to errors
 *      - on_stack_sync is called when host has synced with controller
 */
static void on_stack_reset(int reason) {
    /* On reset, print reset reason to console */
    ESP_LOGI(TAG, "nimble stack reset, reset reason: %d", reason);
}

static void on_stack_sync(void) {
    /* On stack sync, do advertising initialization */
    adv_init();
}
static void nimble_host_config_init(void) {
    /* Set host callbacks */
    ble_hs_cfg.reset_cb = on_stack_reset;
    ble_hs_cfg.sync_cb = on_stack_sync;
    ble_hs_cfg.gatts_register_cb = nullptr;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    /* Store host configuration */
    ble_store_config_init();
}
static void nimble_host_task(void* param) {
    /* Task entry log */
    ESP_LOGI(TAG, "nimble host task has been started!");

    /* This function won't return until nimble_port_stop() is executed */
    nimble_port_run();

    /* Clean up at exit */
    vTaskDelete(NULL);
}

plant::BLEServiceRegistrator ble_services;
plant::BLECurrentTimeService current_time_service;
plant::BLEBatteryService battery_service;
plant::BLEWateringService watering_service;
plant::BLENameService name_service;

extern "C" void app_main(void) {
    {
        auto ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
            ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
        }
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "failed to initialize nvs flash, error code: %d ",
                     ret);
            return;
        }
    }

    ManufacturerData manufacturer_data;

    if (auto saved_name = plant::read_user_defined_name_from_storage()) {
        ESP_LOGI(TAG, "Name read from storage");
        manufacturer_data.part2.user_defined_name = saved_name->as_array();
    }

    if (auto ret = nimble_port_init(); ret != ESP_OK) {
        ESP_LOGE(TAG, "failed to initialize nimble stack, error code: %d ", ret);
        return;
    }

    if (auto error_code = gap_init(manufacturer_data); error_code != 0) {
        ESP_LOGE(TAG, "failed to initialize GAP service, error code: %d",
                 error_code);
        return;
    }

    ble_services.add_service(current_time_service);
    ble_services.add_service(battery_service);
    ble_services.add_service(watering_service);
    ble_services.add_service(name_service);

    name_service.name().set(manufacturer_data.part2.user_defined_name);

    ble_services.register_all_services();

    if (not ble_services.registration_done()) {
        ESP_LOGE(TAG, "failed to register BLE services");
        return;
    }

    nimble_host_config_init();

    QueueHandle_t hardware_queue =
        xQueueCreate(1, sizeof(plant::HardwareState));

    QueueHandle_t watering_requests_queue =
        xQueueCreate(10, sizeof(plant::WateringRequest));

    QueueHandle_t watering_schedule_queue =
        xQueueCreate(10, sizeof(plant::WateringProgram));

    plant::create_hardware_task(hardware_queue);

    auto* watering_task = plant::create_watering_task(
        {.watering_queue = watering_requests_queue,
         .hardware_queue = hardware_queue,
         .watering_state_characteristic = &watering_service.watering_state()});

    plant::create_schedule_task(
        {.watering_schedule_queue = watering_schedule_queue,
         .watering_requests_queue = watering_requests_queue});

    watering_service.watering_test().set_watering_requests_queue(
        watering_requests_queue);
    watering_service.watering_test().set_watering_task_handle(watering_task);

    watering_service.watering_schedule().set_watering_schedule_queue(
        watering_schedule_queue);

    xTaskCreate(nimble_host_task, "NimBLE Host", 4 * 1024, nullptr, 5, nullptr);
}
