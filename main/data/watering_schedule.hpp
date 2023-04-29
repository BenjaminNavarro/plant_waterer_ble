#pragma once

#include <array>
#include <cstdint>

namespace plant {

struct PlantGroupSchedule {
    bool enabled{};
    std::uint64_t start_time{};
    std::uint64_t watering_period{};
    std::uint64_t watering_duration{};
};

using WateringSchedule = std::array<PlantGroupSchedule, 8>;

} // namespace plant