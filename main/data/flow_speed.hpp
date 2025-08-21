#pragma once

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

} // namespace plant