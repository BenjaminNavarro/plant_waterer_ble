#pragma once

#include <driver/gpio.h>

namespace plant {

class Relay {
public:
    Relay(gpio_num_t output) : output_{output} {
        gpio_reset_pin(output_);
        gpio_set_direction(output_, GPIO_MODE_OUTPUT);
    }

    void open() {
        gpio_set_level(output_, 1);
        state_ = false;
    }

    void close() {
        gpio_set_level(output_, 0);
        state_ = true;
    }

    [[nodiscard]] bool is_closed() const {
        return state_;
    }

    [[nodiscard]] bool is_open() const {
        return not is_closed();
    }

private:
    gpio_num_t output_;
    bool state_{};
};

} // namespace plant