#include <data/watering_schedule.hpp>

#include <nvs.h>

namespace plant {

std::optional<WateringSchedule> read_schedule_from_storage() {
    nvs_handle_t nvs_handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &nvs_handle));

    // Read the size of memory space required for blob
    size_t required_size = 0; // value will default to 0, if not set yet in NVS
    esp_err_t err =
        nvs_get_blob(nvs_handle, "schedule", nullptr, &required_size);
    ESP_ERROR_CHECK(err == ESP_OK or err == ESP_ERR_NVS_NOT_FOUND ? ESP_OK
                                                                  : ESP_FAIL);
    if (required_size == 0) {
        nvs_close(nvs_handle);
        return {};
    } else if (required_size != sizeof(WateringSchedule)) {
        ESP_ERROR_CHECK(ESP_FAIL);
    }

    WateringSchedule schedule;
    ESP_ERROR_CHECK(
        nvs_get_blob(nvs_handle, "schedule", &schedule, &required_size));

    nvs_close(nvs_handle);
    return schedule;
}

void write_schedule_from_storage(const WateringSchedule& schedule) {
    nvs_handle_t nvs_handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &nvs_handle));

    ESP_ERROR_CHECK(
        nvs_set_blob(nvs_handle, "schedule", &schedule, sizeof(schedule)));

    ESP_ERROR_CHECK(nvs_commit(nvs_handle));

    nvs_close(nvs_handle);
}

} // namespace plant