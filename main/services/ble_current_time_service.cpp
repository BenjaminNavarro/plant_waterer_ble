#include <services/ble_current_time_service.hpp>

#include "ble_utils.hpp"

#include <sys/time.h>

#include <cstring>

namespace plant {

namespace {

/* Current time service */
const auto current_time_svc_uuid =
    make_uuid128("87f4d02e-698f-4c46-91f2-5f714c877b0a");

const auto current_time_chr_uuid =
    make_uuid128("21bc4af5-44f0-4a7b-aa36-a110a0ac0ad2");

} // namespace

BLECurrentTimeCharacteristic::BLECurrentTimeCharacteristic()
    : BLECharacteristic{"BLECurrentTime", &current_time_chr_uuid.u,
                        BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
                        sizeof(std::int64_t)} {
}

std::span<const std::byte>
BLECurrentTimeCharacteristic::on_read_access(std::span<std::byte> memory) {
    struct timeval now_tv;
    gettimeofday(&now_tv, nullptr);

    std::memcpy(memory.data(), &now_tv.tv_sec, sizeof(now_tv.tv_sec));

    return memory;
}

void BLECurrentTimeCharacteristic::on_write(std::span<const std::byte> memory) {
    if (memory.size() != sizeof(std::int64_t)) {
        return;
    }

    struct timeval tv;
    tv.tv_usec = 0;

    std::memcpy(&tv.tv_sec, memory.data(), memory.size());

    settimeofday(&tv, nullptr);
}

BLECurrentTimeService::BLECurrentTimeService()
    : BLEServiceWrapper{BLE_GATT_SVC_TYPE_PRIMARY, &current_time_svc_uuid.u} {
}

} // namespace plant