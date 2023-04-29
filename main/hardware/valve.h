#pragma once

#include <hardware/relay.hpp>

namespace plant {

class Valve {
public:
    explicit Valve(Relay relay) : relay_{relay} {
    }

    void open() {
        relay_.close();
    }

    void close() {
        relay_.open();
    }

    [[nodiscard]] bool is_open() const {
        return relay_.is_closed();
    }

private:
    Relay relay_;
};

} // namespace plant