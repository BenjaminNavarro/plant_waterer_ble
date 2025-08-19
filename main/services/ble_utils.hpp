#pragma once

#include "host/ble_uuid.h"

#include <string_view>
#include <cstddef>

namespace plant {

// expected format: 4f736c21-2054-4786-93fe-a5c4b028dbef
constexpr ble_uuid128_t make_uuid128(std::string_view str) {
    ble_uuid128_t uuid;
    uuid.u.type = BLE_UUID_TYPE_128;

    auto hex_char_to_dec = [](char c) -> int {
        if (c >= '0' and c <= '9') {
            return c - '0';
        } else if (c >= 'a' and c <= 'f') {
            return 10 + c - 'a';
        } else if (c >= 'A' and c <= 'F') {
            return 10 + c - 'A';
        } else {
            return -1;
        }
    };

    auto parse_hex_byte = [hex_char_to_dec](std::string_view str) -> int {
        return hex_char_to_dec(str[0]) << 4 | hex_char_to_dec(str[1]);
    };

    for (std::size_t i = 0, idx = 15; i < str.size(); ++i) {
        if (str[i] == '-') {
            continue;
        }

        uuid.value[idx] = parse_hex_byte(str.substr(i, 2));
        --idx;
        ++i;
    }

    return uuid;
}

int gatt_svr_write(struct os_mbuf* om, uint16_t min_len, uint16_t max_len,
                   void* dst, uint16_t* len);

} // namespace plant