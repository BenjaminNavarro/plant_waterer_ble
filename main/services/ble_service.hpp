#pragma once

#include <services/ble_characteristic.hpp>

#include <array>
#include <vector>
#include <cstdint>

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
        : BLEService{type, uuid} {
        set_characteristics(characteristics);
    }

    BLEService(uint8_t type, const ble_uuid_t* uuid)
        : service_{.type = type,
                   .uuid = uuid,
                   .includes = nullptr,
                   .characteristics = characteristics_.data()} {
    }

    void
    set_characteristics(const std::array<ble_gatt_chr_def, N>& characteristics) {
        ESP_LOGI("BLEService", "type=%d, uuid=%d, characteristics count=%d",
                 service_.type, service_.uuid->type, characteristics.size());
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

protected:
    friend class BLEServiceRegistrator;
    virtual void service_added() {
    }

private:
    std::array<ble_gatt_chr_def, N + 1> characteristics_;
    ble_gatt_svc_def service_;
};

template <typename... Characteristics>
class BLEServiceWrapper : public BLEService<sizeof...(Characteristics)> {
public:
    BLEServiceWrapper(uint8_t type, const ble_uuid_t* uuid)
        : BLEService<sizeof...(Characteristics)>{type, uuid} {
        set_characteristics();
    }

    BLEServiceWrapper(uint8_t type, const ble_uuid_t* uuid,
                      Characteristics&&... args)
        : BLEService<sizeof...(Characteristics)>{type, uuid},
          characteristics_{std::move(args)...} {
        set_characteristics();
    }

    template <int N> [[nodiscard]] const auto& get_characteristic() const {
        return std::get<N>(characteristics_);
    }

    template <int N> [[nodiscard]] auto& get_characteristic() {
        return std::get<N>(characteristics_);
    }

private:
    void set_characteristics() {
        std::array<ble_gatt_chr_def, sizeof...(Characteristics)> chr_array;
        int idx{};

        auto set_next = [&](const auto& chr) {
            chr_array[idx++] = chr.settings();
        };

        std::apply([&](const auto&... chr) { (set_next(chr), ...); },
                   characteristics_);

        BLEService<sizeof...(Characteristics)>::set_characteristics(chr_array);
    }

    std::tuple<Characteristics...> characteristics_{};
};

class BLEServiceRegistrator {
public:
    template <int N> void add_service(BLEService<N>& service) {
        add_service(service.service());
        service.service_added();
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