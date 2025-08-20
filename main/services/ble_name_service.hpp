#pragma once

#include <services/ble_service.hpp>
#include <data/user_defined_name.hpp>

#include <host/ble_gatt.h>

#include <esp_err.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

namespace plant {

class BLENameCharacteristic
    : public BLECharacteristic<ManufacturerData::Part2::name_max_size> {
public:
    BLENameCharacteristic();

    [[nodiscard]] const UserDefinedName& name() const {
        return name_;
    }

    [[nodiscard]] UserDefinedName& name() {
        return name_;
    };

private:
    [[nodiscard]] std::span<const std::byte>
    on_read_access(std::span<std::byte> memory) override final;

    void on_write(std::span<const std::byte> memory) override final;

    UserDefinedName name_;
};

class BLENameService : public BLEServiceWrapper<BLENameCharacteristic> {
public:
    BLENameService();

    [[nodiscard]] const UserDefinedName& name() const {
        return get_characteristic<0>().name();
    }

    [[nodiscard]] UserDefinedName& name() {
        return get_characteristic<0>().name();
    };
};

} // namespace plant