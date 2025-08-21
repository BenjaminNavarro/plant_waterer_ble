#pragma once

#include <data/hardware_state.hpp>
#include <services/ble_watering_service/ble_watering_service.hpp>

namespace plant {

struct WateringProgram {
    WateringSchedule schedule;
    std::uint8_t output{};
};

} // namespace plant