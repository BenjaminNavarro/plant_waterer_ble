#include <services/ble_service.hpp>

#include <esp_log.h>
#include <host/ble_hs.h>
#include <services/gatt/ble_svc_gatt.h>

namespace plant {

void BLEServiceRegistrator::add_service(const ble_gatt_svc_def& service) {
    if (registration_done_) {
        ESP_LOGI("BLEServiceRegistrator",
                 "Cannot add a new service after registration has been done");
        return;
    }

    services_.push_back(service);
}

void BLEServiceRegistrator::register_all_services() {
    if (registration_done_) {
        ESP_LOGI("BLEServiceRegistrator",
                 "services already registered, doing nothing");
        return;
    }

    services_.push_back(ble_gatt_svc_def{.type = 0});

    /* 1. GATT service initialization */
    ble_svc_gatt_init();

    /* 2. Update GATT services counter */
    int rc = ble_gatts_count_cfg(services_.data());
    if (rc != 0) {
        ESP_LOGI("BLEServiceRegistrator", "ble_gatts_count_cfg() failed, rc=%d",
                 rc);
        return;
    }

    /* 3. Add GATT services */
    rc = ble_gatts_add_svcs(services_.data());
    if (rc != 0) {
        ESP_LOGI("BLEServiceRegistrator", "ble_gatts_add_svcs() failed, rc=%d",
                 rc);
        return;
    }

    registration_done_ = true;
    ESP_LOGI("BLEServiceRegistrator", "all %d services registered",
             services_.size() - 1);
}

} // namespace plant
