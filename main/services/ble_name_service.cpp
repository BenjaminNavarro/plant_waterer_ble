#include <cstring>
#include <services/ble_name_service.hpp>

#include "ble_utils.hpp"

namespace plant {

namespace {

/* Custom name service */
constexpr auto name_svc_uuid =
    make_uuid128("4f736c21-2054-4786-93fe-a5c4b028dbef");

const auto name_chr_uuid = make_uuid128("b8b4c3af-fa31-4de4-9fa1-a26ea5da7f0b");

} // namespace

BLENameCharacteristic::BLENameCharacteristic()
    : BLECharacteristic{"BLEName", &name_chr_uuid.u,
                        BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE} {
}

std::span<const std::byte>
BLENameCharacteristic::on_read_access(std::span<std::byte> memory) {
    const auto value = name_.as_array();

    std::memcpy(memory.data(), value.data(), value.size());

    return memory;
}

void BLENameCharacteristic::on_write(std::span<const std::byte> memory) {
    auto value = name().as_array();
    std::memcpy(value.data(), memory.data(), memory.size());
    name().set(value);
    write_user_defined_name_to_storage(name());

    {
        auto mfg_data = getCurrentManufacturerData();
        mfg_data.part2.user_defined_name = name().as_array();
        updateManufacturerData(mfg_data);
    }

    ESP_LOGI("BLENameService", "New device name: %s\n", name().get().data());
}

BLENameService::BLENameService()
    : BLEServiceWrapper{BLE_GATT_SVC_TYPE_PRIMARY, &name_svc_uuid.u} {
}

} // namespace plant