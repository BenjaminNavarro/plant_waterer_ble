/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* Includes */
#include <services/ble_service.hpp>
#include <services/ble_current_time_service.hpp>
#include <services/ble_battery_service.hpp>
#include <services/ble_watering_service.hpp>

#include "common.h"
#include "gap.hpp"
#include "gatt_svc.hpp"

/* Library function declarations */
extern "C" void ble_store_config_init(void);

/* Private function declarations */
static void on_stack_reset(int reason);
static void on_stack_sync(void);
static void nimble_host_config_init(void);
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
    ble_hs_cfg.gatts_register_cb = gatt_svr_register_cb;
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

static void current_time_task(void* param) {
    /* Task entry log */
    ESP_LOGI(TAG, "heart rate task has been started!");

    /* Loop forever */
    while (1) {
        /* Send heart rate indication if enabled */
        send_current_time_indication();

        /* Sleep */
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    /* Clean up at exit */
    vTaskDelete(nullptr);
}

plant::BLEServiceRegistrator ble_services;
plant::BLECurrentTimeService current_time_service;
plant::BLEBatteryService battery_service;
plant::BLEWateringService watering_service;

void app_main_ble() {
    /* Local variables */
    int rc;
    esp_err_t ret;

    /* LED initialization */
    // led_init();

    /*
     * NVS flash initialization
     * Dependency of BLE stack to store configurations
     */
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "failed to initialize nvs flash, error code: %d ", ret);
        return;
    }

    /* NimBLE stack initialization */
    ret = nimble_port_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "failed to initialize nimble stack, error code: %d ", ret);
        return;
    }

    /* GAP service initialization */
    ManufacturerData manufacturer_data;
    rc = gap_init(manufacturer_data);
    if (rc != 0) {
        ESP_LOGE(TAG, "failed to initialize GAP service, error code: %d", rc);
        return;
    }

    ble_services.add_service(current_time_service);
    ble_services.add_service(battery_service);
    ble_services.add_service(watering_service);

    ble_services.register_all_services();

    if (not ble_services.registration_done()) {
        ESP_LOGE(TAG, "failed to register BLE services");
        return;
    }

    /* NimBLE host configuration initialization */
    nimble_host_config_init();

    /* Start NimBLE host task thread and return */
    xTaskCreate(nimble_host_task, "NimBLE Host", 4 * 1024, NULL, 5, NULL);
    // xTaskCreate(current_time_task, "Heart Rate", 4 * 1024, NULL, 5, NULL);
}
