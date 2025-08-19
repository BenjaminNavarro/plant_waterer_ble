#pragma once

#include <services/ble_service.hpp>

#include <host/ble_gatt.h>

#include <esp_err.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include <cstdint>

namespace plant {

class BLECurrentTimeService : public BLEService<1> {
public:
    BLECurrentTimeService();

private:
    static int current_time_chr_access(uint16_t conn_handle,
                                       uint16_t attr_handle,
                                       struct ble_gatt_access_ctxt* ctxt,
                                       void* arg);
};

} // namespace plant