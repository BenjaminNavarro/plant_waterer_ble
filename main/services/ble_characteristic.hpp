#pragma once

#include <array>
#include <cstddef>
#include <string_view>
#include <span>
#include <cstdint>

#include <host/ble_gatt.h>

#include <esp_err.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

namespace plant {

class BLECharacteristicBase {
public:
    [[nodiscard]] const ble_gatt_chr_def& settings() const {
        return characteristic_settings_;
    }

    [[nodiscard]] const std::string_view& name() const {
        return name_;
    }

protected:
    BLECharacteristicBase(std::string_view name, const ble_uuid_t* uuid,
                          ble_gatt_chr_flags flags,
                          std::span<std::byte> read_memory,
                          std::span<std::byte> write_memory,
                          std::uint16_t min_packet_size);

    //! \param memory Memory area to write to
    //! \return std::span<const std::byte> (sub)span of memory to send to the device
    virtual std::span<const std::byte>
    on_read_access(std::span<std::byte> memory) {
        return memory;
    }

    virtual void on_read() {
    }

    virtual void on_write(std::span<const std::byte> memory) {
    }

    void send_update_notification() const;

private:
    static int on_access(uint16_t conn_handle, uint16_t attr_handle,
                         ble_gatt_access_ctxt* ctxt, void* arg);

    ble_gatt_chr_def characteristic_settings_{};
    uint16_t characteristic_value_handle_{};
    std::string_view name_;
    std::span<std::byte> read_memory_;
    std::span<std::byte> write_memory_;
    std::uint16_t min_packet_size_;
};

template <int MaxPacketSize, int MinWriteSize = MaxPacketSize>
class BLECharacteristic : public BLECharacteristicBase {
public:
    BLECharacteristic(std::string_view name, const ble_uuid_t* uuid,
                      ble_gatt_chr_flags flags)
        : BLECharacteristicBase{name,         uuid,          flags,
                                read_buffer_, write_buffer_, MinWriteSize} {
        read_buffer_.fill(std::byte(0));
        write_buffer_.fill(std::byte(0));
    }

private:
    std::array<std::byte, MaxPacketSize> read_buffer_;
    std::array<std::byte, MaxPacketSize> write_buffer_;
};

} // namespace plant