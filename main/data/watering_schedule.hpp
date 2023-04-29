#pragma once

#include <array>
#include <optional>
#include <cstdint>

namespace plant {

struct PlantGroupSchedule {
    bool enabled{};
    std::uint64_t start_time{};
    std::uint64_t watering_period{};
    std::uint64_t watering_duration{};
};

using WateringSchedule = std::array<PlantGroupSchedule, 8>;

std::optional<WateringSchedule> read_schedule_from_storage();
void write_schedule_from_storage(const WateringSchedule& schedule);

} // namespace plant