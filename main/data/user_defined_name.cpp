#include <data/user_defined_name.hpp>

#include <nvs.h>

namespace plant {

std::optional<UserDefinedName> read_user_defined_name_from_storage() {
    nvs_handle_t nvs_handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &nvs_handle));

    // Read the size of memory space required for blob
    size_t required_size = 0; // value will default to 0, if not set yet in NVS
    esp_err_t err = nvs_get_blob(nvs_handle, "name", nullptr, &required_size);
    ESP_ERROR_CHECK(err == ESP_OK or err == ESP_ERR_NVS_NOT_FOUND ? ESP_OK
                                                                  : ESP_FAIL);
    if (required_size != sizeof(UserDefinedName)) {
        nvs_close(nvs_handle);
        return {};
    }

    UserDefinedName name;
    ESP_ERROR_CHECK(nvs_get_blob(nvs_handle, "name", &name, &required_size));

    nvs_close(nvs_handle);
    return name;
}

void write_user_defined_name_to_storage(const UserDefinedName& name) {
    nvs_handle_t nvs_handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &nvs_handle));

    ESP_ERROR_CHECK(nvs_set_blob(nvs_handle, "name", &name, sizeof(name)));

    ESP_ERROR_CHECK(nvs_commit(nvs_handle));

    nvs_close(nvs_handle);
}

} // namespace plant