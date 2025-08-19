#include "ble_utils.hpp"

#include <host/ble_hs.h>

int plant::gatt_svr_write(struct os_mbuf* om, uint16_t min_len,
                          uint16_t max_len, void* dst, uint16_t* len) {
    uint16_t om_len;
    int rc;

    om_len = OS_MBUF_PKTLEN(om);
    if (om_len < min_len || om_len > max_len) {
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }

    rc = ble_hs_mbuf_to_flat(om, dst, max_len, len);
    if (rc != 0) {
        return BLE_ATT_ERR_UNLIKELY;
    }

    return 0;
}