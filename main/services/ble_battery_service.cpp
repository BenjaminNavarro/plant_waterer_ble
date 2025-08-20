#include <cstring>
#include <services/ble_battery_service.hpp>

namespace plant {

namespace {

const ble_uuid16_t battery_svc_uuid = BLE_UUID16_INIT(0x180F);
const ble_uuid16_t battery_level_chr_uuid = BLE_UUID16_INIT(0x2A19);

} // namespace

BLEBatteryCharacteristic::BLEBatteryCharacteristic()
    : BLECharacteristic{"BLEBattery", &battery_level_chr_uuid.u,
                        BLE_GATT_CHR_F_READ},
      battery_level_{1.} {
}

std::span<const std::byte>
BLEBatteryCharacteristic::on_read_access(std::span<std::byte> memory) {
    const auto value = static_cast<std::uint8_t>(battery_level() * 100.f);
    battery_level() *= 0.9;

    memory[0] = std::byte{value};

    return memory;
}

BLEBatteryService::BLEBatteryService()
    : BLEServiceWrapper{BLE_GATT_SVC_TYPE_PRIMARY, &battery_svc_uuid.u} {
}

} // namespace plant