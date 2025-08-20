#pragma once

#include <services/ble_service.hpp>

#include <host/ble_gatt.h>

#include <esp_err.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <cstdint>

namespace plant {

// Make sure the data is packed
struct WateringSchedule {
    std::uint64_t start_time{};        // UNIX time
    std::uint32_t watering_period{};   // in seconds
    std::uint16_t watering_duration{}; // in seconds
    std::uint8_t flow_speed{};         // 0-100
    bool enabled{};

    static const auto size = sizeof(start_time) + sizeof(watering_period) +
                             sizeof(watering_duration) + sizeof(flow_speed) +
                             sizeof(enabled);

    void read_from_storage();
    void write_to_storage() const;
};

// Make sure the data is packed
struct WateringTest {
    std::uint16_t duration{10};  // in seconds
    std::uint8_t flow_speed{30}; // 0-100

    static const auto size = sizeof(duration) + sizeof(flow_speed);
};

// Make sure the data is packed
struct WateringState {
    std::uint64_t start_time{}; // UNIX time
    std::uint16_t watering_duration{};
    bool watering{};

    static const auto size =
        sizeof(start_time) + sizeof(watering_duration) + sizeof(watering);
};

class BLEWateringScheduleCharacteristic
    : public BLECharacteristic<sizeof(WateringSchedule)> {
public:
    BLEWateringScheduleCharacteristic();

    [[nodiscard]] const WateringSchedule& schedule() const {
        return schedule_;
    }

    [[nodiscard]] WateringSchedule& schedule() {
        return schedule_;
    };

private:
    [[nodiscard]] std::span<const std::byte>
    on_read_access(std::span<std::byte> memory) override final;

    void on_write(std::span<const std::byte> memory) override final;

    WateringSchedule schedule_;
};

class BLEWateringTestCharacteristic
    : public BLECharacteristic<sizeof(WateringTest)> {
public:
    BLEWateringTestCharacteristic();

    [[nodiscard]] const WateringTest& test() const {
        return test_;
    }

    [[nodiscard]] WateringTest& test() {
        return test_;
    };

private:
    void on_write(std::span<const std::byte> memory) override final;

    WateringTest test_;
};

class BLEWateringStateCharacteristic
    : public BLECharacteristic<sizeof(WateringState)> {
public:
    BLEWateringStateCharacteristic();

    [[nodiscard]] const WateringState& state() const {
        return state_;
    }

    void set_state(const WateringState& state) {
        state_ = state;
        notify_watering_state_change();
    };

private:
    [[nodiscard]] std::span<const std::byte>
    on_read_access(std::span<std::byte> memory) override final;

    void notify_watering_state_change();

    WateringState state_;
};

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
        return get_characteristic<0>();
    }

    [[nodiscard]] auto& watering_state() {
        return get_characteristic<0>();
    }

private:
    friend class BLEServiceRegistrator;

    void service_added() final override;
};

} // namespace plant