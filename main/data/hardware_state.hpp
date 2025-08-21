#pragma once

#include <array>
#include <cstdint>

namespace plant {

constexpr std::size_t valve_count = 1;

struct HardwareState {
    HardwareState() {
        output_state.fill(false);
        flow_speed = 0;
    }

    std::array<bool, valve_count> output_state;
    std::uint8_t flow_speed; // 0-100

    [[nodiscard]] constexpr bool operator==(const HardwareState& other) const {
        return output_state == other.output_state and
               flow_speed == other.flow_speed;
    }

    [[nodiscard]] constexpr bool operator!=(const HardwareState& other) const {
        return not(*this == other);
    }
};

} // namespace plant