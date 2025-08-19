#pragma once

#include <services/ble_service.hpp>
#include <data/user_defined_name.hpp>

#include <host/ble_gatt.h>

#include <esp_err.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

namespace plant {

class BLENameService : public BLEService<1> {
public:
    BLENameService();

    [[nodiscard]] const UserDefinedName& name() const {
        return name_;
    }

    [[nodiscard]] UserDefinedName& name() {
        return name_;
    };

private:
    static int name_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt* ctxt, void* arg);

    UserDefinedName name_;
};

} // namespace plant