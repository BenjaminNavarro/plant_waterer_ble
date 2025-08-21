/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#ifndef GAP_SVC_H
#define GAP_SVC_H

/* Includes */
/* NimBLE GAP APIs */
#include "host/ble_gap.h"
#include "services/gap/ble_svc_gap.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <string_view>

/* Defines */
#define BLE_GAP_APPEARANCE_GENERIC_TAG 0x0200
#define BLE_GAP_URI_PREFIX_HTTPS 0x17
#define BLE_GAP_LE_ROLE_PERIPHERAL 0x00

constexpr std::uint16_t mfg_id_from_string(std::string_view id) {
    if (id.size() != 2) {
        return 0;
    }

    return ((std::uint16_t(id[1]) & 0xFF) << 8) | (std::uint16_t(id[0]) & 0xFF);
}

struct ManufacturerData {
    struct Part1 {
        // 10 bytes available in the advertizing buffer for manufacturer data
        static constexpr std::size_t max_size = 10;

        std::uint16_t id = mfg_id_from_string("HW");
        std::uint8_t output_count = 1;

        std::array<uint8_t, max_size> to_array() {
            std::array<uint8_t, max_size> array;
            array.fill(0);

            std::size_t idx{};
            array[idx++] = (id & 0xFF00) >> 8;
            array[idx++] = id & 0xFF;
            array[idx++] = output_count;

            return array;
        }
    } part1;

    struct Part2 {
        // 16 bytes available in the scan response buffer for manufacturer data
        static constexpr std::size_t max_size = 16;
        static constexpr std::size_t name_max_size = max_size - 2;

        static constexpr std::uint16_t id = mfg_id_from_string("TD");
        std::array<char, name_max_size> user_defined_name =
            std::to_array("My Cute Plant");

        std::array<uint8_t, max_size> to_array() {
            std::array<uint8_t, max_size> array;
            array.fill(0);

            std::size_t idx{};
            array[idx++] = (id & 0xFF00) >> 8;
            array[idx++] = id & 0xFF;
            std::copy(user_defined_name.begin(), user_defined_name.end(),
                      array.begin() + idx);

            return array;
        }
    } part2;

    static_assert(sizeof(Part1) <= Part1::max_size);
    static_assert(sizeof(Part2) <= Part2::max_size);
};

/* Public function declarations */
void adv_init();
int gap_init(const ManufacturerData& manufacturer_data);

ManufacturerData getCurrentManufacturerData();
void updateManufacturerData(const ManufacturerData& manufacturer_data);

#endif // GAP_SVC_H
