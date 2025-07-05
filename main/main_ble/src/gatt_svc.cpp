/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* Includes */
#include "gatt_svc.hpp"
#include "common.h"
#include "led.hpp"
#include "services/cts/ble_svc_cts.h"

#include <vector>

/* Private function declarations */
static int current_time_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                                   struct ble_gatt_access_ctxt* ctxt, void* arg);
static int led_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                          struct ble_gatt_access_ctxt* ctxt, void* arg);

/* Private variables */
/* Current time service */
static const ble_uuid16_t current_time_svc_uuid = BLE_UUID16_INIT(0x1805);

static uint32_t current_time_chr_val = {0};
static uint16_t current_time_chr_val_handle;
static const ble_uuid16_t current_time_chr_uuid = BLE_UUID16_INIT(0x2A2B);

static uint16_t current_time_chr_conn_handle = 0;
static bool current_time_chr_conn_handle_inited = false;
static bool current_time_ind_status = false;

struct BLECurrentTime {
    // https://iot.stackexchange.com/a/7887
    uint16_t year{2025};
    uint8_t month{5};
    uint8_t day{30};
    uint8_t hours{19};
    uint8_t minutes{01};
    uint8_t seconds{10};
    uint8_t day_of_week{1};
    uint8_t fractions_256{0};
    uint8_t adjust_reason{0x03}; // manual update
};

BLECurrentTime ble_current_time;

/* Automation IO service */
static const ble_uuid16_t auto_io_svc_uuid = BLE_UUID16_INIT(0x1815);
static uint16_t led_chr_val_handle;
static const ble_uuid128_t led_chr_uuid =
    BLE_UUID128_INIT(0x23, 0xd1, 0xbc, 0xea, 0x5f, 0x78, 0x23, 0x15, 0xde, 0xef,
                     0x12, 0x12, 0x25, 0x15, 0x00, 0x00);

/* GATT services table */
static const ble_gatt_svc_def gatt_svr_svcs[] = {
    /* Heart rate service */
    {.type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = &current_time_svc_uuid.u,
     .characteristics =
         (ble_gatt_chr_def[]){
             {/* Heart rate characteristic */
              .uuid = &current_time_chr_uuid.u,
              .access_cb = current_time_chr_access,
              .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
              .val_handle = &current_time_chr_val_handle},
             {
                 .uuid = 0, /* No more characteristics in this service. */
             }}},

    /* Automation IO service */
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &auto_io_svc_uuid.u,
        .characteristics =
            (ble_gatt_chr_def[]){/* LED characteristic */
                                 {.uuid = &led_chr_uuid.u,
                                  .access_cb = led_chr_access,
                                  .flags = BLE_GATT_CHR_F_WRITE,
                                  .val_handle = &led_chr_val_handle},
                                 {0}},
    },

    {
        .type = 0, /* No more services. */
    },
};

/* Private functions */
static int current_time_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                                   struct ble_gatt_access_ctxt* ctxt,
                                   void* arg) {
    /* Local variables */
    int rc;

    /* Handle access events */
    /* Note: Heart rate characteristic is read only */
    switch (ctxt->op) {

    /* Read characteristic event */
    case BLE_GATT_ACCESS_OP_READ_CHR:
        /* Verify connection handle */
        if (conn_handle != BLE_HS_CONN_HANDLE_NONE) {
            ESP_LOGI(TAG, "characteristic read; conn_handle=%d attr_handle=%d",
                     conn_handle, attr_handle);
        } else {
            ESP_LOGI(TAG, "characteristic read by nimble stack; attr_handle=%d",
                     attr_handle);
        }

        /* Verify attribute handle */
        if (attr_handle == current_time_chr_val_handle) {
            ble_current_time.seconds =
                (ble_current_time.seconds +
                 xTaskGetTickCount() / xPortGetTickRateHz()) %
                60;
            rc = os_mbuf_append(ctxt->om, &ble_current_time,
                                sizeof(ble_current_time));
            return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        }
        goto error;

    /* Unknown event */
    default:
        goto error;
    }

error:
    ESP_LOGE(
        TAG,
        "unexpected access operation to heart rate characteristic, opcode: %d",
        ctxt->op);
    return BLE_ATT_ERR_UNLIKELY;
}

static int led_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                          struct ble_gatt_access_ctxt* ctxt, void* arg) {
    /* Local variables */
    int rc;

    /* Handle access events */
    /* Note: LED characteristic is write only */
    switch (ctxt->op) {

    /* Write characteristic event */
    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        /* Verify connection handle */
        if (conn_handle != BLE_HS_CONN_HANDLE_NONE) {
            ESP_LOGI(TAG, "characteristic write; conn_handle=%d attr_handle=%d",
                     conn_handle, attr_handle);
        } else {
            ESP_LOGI(TAG, "characteristic write by nimble stack; attr_handle=%d",
                     attr_handle);
        }

        /* Verify attribute handle */
        if (attr_handle == led_chr_val_handle) {
            /* Verify access buffer length */
            if (ctxt->om->om_len == 1) {
                /* Turn the LED on or off according to the operation bit */
                if (ctxt->om->om_data[0]) {
                    led_on();
                    ESP_LOGI(TAG, "led turned on!");
                } else {
                    led_off();
                    ESP_LOGI(TAG, "led turned off!");
                }
            } else {
                goto error;
            }
            return rc;
        }
        goto error;

    /* Unknown event */
    default:
        goto error;
    }

error:
    ESP_LOGE(TAG,
             "unexpected access operation to led characteristic, opcode: %d",
             ctxt->op);
    return BLE_ATT_ERR_UNLIKELY;
}

/* Public functions */
void send_current_time_indication(void) {
    if (current_time_ind_status && current_time_chr_conn_handle_inited) {
        ble_gatts_indicate(current_time_chr_conn_handle,
                           current_time_chr_val_handle);
        ESP_LOGI(TAG, "heart rate indication sent!");
    }
}

/*
 *  Handle GATT attribute register events
 *      - Service register event
 *      - Characteristic register event
 *      - Descriptor register event
 */
void gatt_svr_register_cb(struct ble_gatt_register_ctxt* ctxt, void* arg) {
    /* Local variables */
    char buf[BLE_UUID_STR_LEN];

    /* Handle GATT attributes register events */
    switch (ctxt->op) {

    /* Service register event */
    case BLE_GATT_REGISTER_OP_SVC:
        ESP_LOGD(TAG, "registered service %s with handle=%d",
                 ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
                 ctxt->svc.handle);
        break;

    /* Characteristic register event */
    case BLE_GATT_REGISTER_OP_CHR:
        ESP_LOGD(TAG,
                 "registering characteristic %s with "
                 "def_handle=%d val_handle=%d",
                 ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                 ctxt->chr.def_handle, ctxt->chr.val_handle);
        break;

    /* Descriptor register event */
    case BLE_GATT_REGISTER_OP_DSC:
        ESP_LOGD(TAG, "registering descriptor %s with handle=%d",
                 ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                 ctxt->dsc.handle);
        break;

    /* Unknown event */
    default:
        assert(0);
        break;
    }
}

/*
 *  GATT server subscribe event callback
 *      1. Update heart rate subscription status
 */

void gatt_svr_subscribe_cb(struct ble_gap_event* event) {
    /* Check connection handle */
    if (event->subscribe.conn_handle != BLE_HS_CONN_HANDLE_NONE) {
        ESP_LOGI(TAG, "subscribe event; conn_handle=%d attr_handle=%d",
                 event->subscribe.conn_handle, event->subscribe.attr_handle);
    } else {
        ESP_LOGI(TAG, "subscribe by nimble stack; attr_handle=%d",
                 event->subscribe.attr_handle);
    }

    /* Check attribute handle */
    if (event->subscribe.attr_handle == current_time_chr_val_handle) {
        /* Update heart rate subscription status */
        current_time_chr_conn_handle = event->subscribe.conn_handle;
        current_time_chr_conn_handle_inited = true;
        current_time_ind_status = event->subscribe.cur_indicate;
    }
}

static std::vector<ble_gatt_svc_def> gatt_svr_svcs_vec;

/*
 *  GATT server initialization
 *      1. Initialize GATT service
 *      2. Update NimBLE host GATT services counter
 *      3. Add GATT services to server
 */
int gatt_svc_init(void) {
    /* Local variables */
    int rc;

    /* 1. GATT service initialization */
    ble_svc_gatt_init();

    gatt_svr_svcs_vec.reserve(3);
    gatt_svr_svcs_vec.push_back(gatt_svr_svcs[0]);
    gatt_svr_svcs_vec.push_back(gatt_svr_svcs[1]);
    gatt_svr_svcs_vec.push_back(gatt_svr_svcs[2]);

    /* 2. Update GATT services counter */
    rc = ble_gatts_count_cfg(gatt_svr_svcs_vec.data());
    if (rc != 0) {
        return rc;
    }

    /* 3. Add GATT services */
    rc = ble_gatts_add_svcs(gatt_svr_svcs_vec.data());
    if (rc != 0) {
        return rc;
    }

    return 0;
}
