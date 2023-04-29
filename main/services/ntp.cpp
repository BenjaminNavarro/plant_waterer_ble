#include <services/ntp.hpp>

#include <esp_log.h>
#include <esp_netif_sntp.h>
#include <esp_sntp.h>

#include <ctime>

namespace plant {

namespace {
TaskHandle_t task_to_notify{nullptr};
}

void time_sync_notification_cb(struct timeval* timeval) {
    const time_t time = timeval->tv_sec;
    struct tm now;
    localtime_r(&time, &now);

    ESP_LOGI("NTP",
             "Notification of a time synchronization event: %d/%d/%d %d:%d:%d",
             now.tm_mday, now.tm_mon + 1, now.tm_year + 1900, now.tm_hour,
             now.tm_min, now.tm_sec);

    if (task_to_notify != nullptr) {
        xTaskNotifyGive(task_to_notify);
    }
}

[[nodiscard]] esp_err_t start_ntp_service(TaskHandle_t to_notify) {
    task_to_notify = to_notify;

    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
    tzset();

    esp_sntp_config_t config{
        .smooth_sync = false,
        .server_from_dhcp = true,
        .wait_for_sync = true,
        .start = true,
        .sync_cb = &time_sync_notification_cb,
        .renew_servers_after_new_IP = false,
        .ip_event_to_renew = ip_event_t::IP_EVENT_STA_GOT_IP,
        .index_of_first_server = 0,
        .num_of_servers = 1,
        .servers = {"pool.ntp.org"},

    };
    esp_netif_sntp_init(&config);

    // sntp_set_time_sync_notification_cb(&time_sync_notification_cb);

    if (esp_netif_sntp_sync_wait(pdMS_TO_TICKS(10000)) != ESP_OK) {
        ESP_LOGI("NTP", "Failed to update system time within 10s timeout");
    }

    return ESP_OK;
}

} // namespace plant