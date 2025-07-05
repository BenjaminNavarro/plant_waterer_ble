#pragma once

#include <array>
#include <vector>

#include <host/ble_gatt.h>

#include <esp_err.h>
#include "esp_log.h"

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

namespace plant {

template <int N> class BLEService {
public:
    BLEService(uint8_t type, const ble_uuid_t* uuid,
               const std::array<ble_gatt_chr_def, N>& characteristics)
        : service_{.type = type,
                   .uuid = uuid,
                   .includes = nullptr,
                   .characteristics = characteristics_.data()} {
        ESP_LOGI("BLEService", "type=%d, uuid=%d, characteristics count=%d",
                 type, uuid->type, characteristics.size());
        std::copy(characteristics.begin(), characteristics.end(),
                  characteristics_.begin());
        characteristics_.back().uuid = nullptr;
        for (const auto& ch : characteristics_) {
            if (ch.uuid != nullptr) {
                ESP_LOGI("BLEService", " - %d", ch.uuid->type);
            } else {
                ESP_LOGI("BLEService", " - nullptr");
            }
        }
    }

    [[nodiscard]] const ble_gatt_svc_def& service() const {
        return service_;
    }

private:
    std::array<ble_gatt_chr_def, N + 1> characteristics_;
    ble_gatt_svc_def service_;
};

class BLEServiceRegistrator {
public:
    template <int N> void add_service(const BLEService<N>& service) {
        add_service(service.service());
    }

    void add_service(const ble_gatt_svc_def& service);

    void register_all_services();

    [[nodiscard]] bool registration_done() const {
        return registration_done_;
    }

private:
    bool registration_done_{false};
    std::vector<ble_gatt_svc_def> services_;
};

} // namespace plant