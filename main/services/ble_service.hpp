#pragma once

#include <array>
#include <cstddef>
#include <vector>
#include <string_view>
#include <span>
#include <cstdint>

#include <host/ble_gatt.h>

#include <esp_err.h>
#include "esp_log.h"

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
                          std::uint16_t min_packet_size)
        : name_{name},
          read_memory_{read_memory},
          write_memory_{write_memory},
          min_packet_size_{min_packet_size} {
        characteristic_settings_.uuid = uuid;
        characteristic_settings_.flags = flags;
        characteristic_settings_.access_cb = on_access;
        characteristic_settings_.arg = this;
        characteristic_settings_.val_handle = &characteristic_value_handle_;
    }

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

template <int MaxPacketSize>
class BLECharacteristic : public BLECharacteristicBase {
public:
    BLECharacteristic(std::string_view name, const ble_uuid_t* uuid,
                      ble_gatt_chr_flags flags,
                      std::uint16_t min_packet_size = 0)
        : BLECharacteristicBase{name,         uuid,          flags,
                                read_buffer_, write_buffer_, min_packet_size} {
        read_buffer_.fill(std::byte(0));
        write_buffer_.fill(std::byte(0));
    }

private:
    std::array<std::byte, MaxPacketSize> read_buffer_;
    std::array<std::byte, MaxPacketSize> write_buffer_;
};

template <int N> class BLEService {
public:
    BLEService(uint8_t type, const ble_uuid_t* uuid,
               const std::array<ble_gatt_chr_def, N>& characteristics)
        : BLEService{type, uuid} {
        set_characteristics(characteristics);
    }

    BLEService(uint8_t type, const ble_uuid_t* uuid)
        : service_{.type = type,
                   .uuid = uuid,
                   .includes = nullptr,
                   .characteristics = characteristics_.data()} {
    }

    void
    set_characteristics(const std::array<ble_gatt_chr_def, N>& characteristics) {
        ESP_LOGI("BLEService", "type=%d, uuid=%d, characteristics count=%d",
                 service_.type, service_.uuid->type, characteristics.size());
        std::copy(characteristics.begin(), characteristics.end(),
                  characteristics_.begin());
        characteristics_.back().uuid = nullptr;
        for (const auto& ch : characteristics_) {
            if (ch.uuid != nullptr) {
                ESP_LOGI("BLEService", " - %d", ch.uuid->type);
            } else {
                ESP_LOGI("BLEService", " - nullptr");
            }
        }
    }

    [[nodiscard]] const ble_gatt_svc_def& service() const {
        return service_;
    }

protected:
    friend class BLEServiceRegistrator;
    virtual void service_added() {
    }

private:
    std::array<ble_gatt_chr_def, N + 1> characteristics_;
    ble_gatt_svc_def service_;
};

template <typename... Characteristics>
class BLEServiceWrapper : public BLEService<sizeof...(Characteristics)> {
public:
    BLEServiceWrapper(uint8_t type, const ble_uuid_t* uuid)
        : BLEService<sizeof...(Characteristics)>{type, uuid} {
        set_characteristics();
    }

    BLEServiceWrapper(uint8_t type, const ble_uuid_t* uuid,
                      Characteristics&&... args)
        : BLEService<sizeof...(Characteristics)>{type, uuid},
          characteristics_{std::move(args)...} {
        set_characteristics();
    }

    template <int N> [[nodiscard]] const auto& get_characteristic() const {
        return std::get<N>(characteristics_);
    }

    template <int N> [[nodiscard]] auto& get_characteristic() {
        return std::get<N>(characteristics_);
    }

private:
    void set_characteristics() {
        std::array<ble_gatt_chr_def, sizeof...(Characteristics)> chr_array;
        int idx{};

        auto set_next = [&](const auto& chr) {
            chr_array[idx++] = chr.settings();
        };

        std::apply([&](const auto&... chr) { (set_next(chr), ...); },
                   characteristics_);

        BLEService<sizeof...(Characteristics)>::set_characteristics(chr_array);
    }

    std::tuple<Characteristics...> characteristics_{};
};

class BLEServiceRegistrator {
public:
    template <int N> void add_service(BLEService<N>& service) {
        add_service(service.service());
        service.service_added();
    }

    void add_service(const ble_gatt_svc_def& service);

    void register_all_services();

    [[nodiscard]] bool registration_done() const {
        return registration_done_;
    }

private:
    bool registration_done_{false};
    std::vector<ble_gatt_svc_def> services_;
};

} // namespace plant