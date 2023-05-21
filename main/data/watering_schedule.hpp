#pragma once

#include <array>
#include <optional>
#include <cstdint>
#include <cassert>

namespace plant {

class FlowSpeed {
public:
    FlowSpeed() = default;

    explicit FlowSpeed(double value) : value_{value} {
        assert((value >= 0. and value <= 1.) &&
               "Flow speed must be in the [0,1] range");
    }

    [[nodiscard]] double value() const {
        return value_;
    }

    explicit operator double() const {
        return value_;
    }

    [[nodiscard]] bool operator==(FlowSpeed other) const {
        return value_ == other.value_;
    }

    [[nodiscard]] bool operator!=(FlowSpeed other) const {
        return value_ != other.value_;
    }

private:
    double value_{};
};

struct PlantGroupSchedule {
    bool enabled{};
    std::uint64_t start_time{};
    std::uint64_t watering_period{};
    std::uint64_t watering_duration{};
    FlowSpeed flow_speed{};
};

using WateringSchedule = std::array<PlantGroupSchedule, 8>;

struct HardwareState {
    std::array<bool, 8> output_state{};
    FlowSpeed flow_speed{};

    [[nodiscard]] constexpr bool operator==(const HardwareState& other) const {
        return output_state == other.output_state and
               flow_speed == other.flow_speed;
    }

    [[nodiscard]] constexpr bool operator!=(const HardwareState& other) const {
        return not(*this == other);
    }
};

struct WateringTest {
    std::size_t output{};
    std::uint64_t duration{};
    FlowSpeed flow_speed{};
};

std::optional<WateringSchedule> read_schedule_from_storage();
void write_schedule_to_storage(const WateringSchedule& schedule);

} // namespace plant