#pragma once

#include <services/ble_service.hpp>
#include <services/ble_watering_service/ble_watering_schedule.hpp>
#include <services/ble_watering_service/ble_watering_test.hpp>
#include <services/ble_watering_service/ble_watering_state.hpp>

#include <host/ble_gatt.h>

#include <esp_err.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

namespace plant {

class BLEWateringService
    : public BLEServiceWrapper<BLEWateringScheduleCharacteristic,
                               BLEWateringTestCharacteristic,
                               BLEWateringStateCharacteristic> {
public:
    BLEWateringService();

    [[nodiscard]] const auto& watering_schedule() const {
        return get_characteristic<0>();
    }

    [[nodiscard]] auto& watering_schedule() {
        return get_characteristic<0>();
    };

    [[nodiscard]] const auto& watering_test() const {
        return get_characteristic<1>();
    }

    [[nodiscard]] auto& watering_test() {
        return get_characteristic<1>();
    }

    [[nodiscard]] const auto& watering_state() const {
        return get_characteristic<2>();
    }

    [[nodiscard]] auto& watering_state() {
        return get_characteristic<2>();
    }

private:
    friend class BLEServiceRegistrator;

    void service_added() final override;
};

} // namespace plant