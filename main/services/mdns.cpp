#include <services/mdns.hpp>

#include <mdns.h>
#include <array>

namespace plant {
esp_err_t start_mdns_service() {
    mdns_init();
    mdns_hostname_set(CONFIG_EXAMPLE_MDNS_HOST_NAME);
    mdns_instance_name_set("esp home web server");

    std::array service_txt_data{mdns_txt_item_t{"board", "esp32"},
                                mdns_txt_item_t{"path", "/"}};

    ESP_ERROR_CHECK(mdns_service_add("ESP32-WebServer", "_http", "_tcp", 80,
                                     service_txt_data.data(),
                                     service_txt_data.size()));

    return ESP_OK;
}
} // namespace plant
