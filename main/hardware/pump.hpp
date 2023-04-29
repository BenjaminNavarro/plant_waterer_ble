#pragma once

#include <hardware/relay.hpp>

namespace plant {

class WaterPump {
public:
    explicit WaterPump(Relay relay) : relay_{relay} {
        disable();
    }

    void enable() {
        relay_.close();
    }

    void disable() {
        relay_.open();
    }

    [[nodiscard]] bool is_enabled() const {
        return relay_.is_closed();
    }

private:
    Relay relay_;
};

} // namespace plant