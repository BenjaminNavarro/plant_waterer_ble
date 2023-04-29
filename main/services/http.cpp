#include <services/http.hpp>

#include <cJSON.h>
#include <esp_chip_info.h>
#include <esp_http_server.h>
#include <esp_log.h>
#include <esp_random.h>
#include <fcntl.h>

#include <array>
#include <cstring>

static const char* rest_tag = "esp-rest";
#define REST_CHECK(a, str, callback, ...)                                      \
    do {                                                                       \
        if (!(a)) {                                                            \
            ESP_LOGE(rest_tag, "%s(%d): " str, __FUNCTION__, __LINE__,         \
                     ##__VA_ARGS__);                                           \
            return callback();                                                 \
        }                                                                      \
    } while (0)

constexpr size_t scratch_bufsize = 10240;

struct RestServerContext {
    std::array<char, scratch_bufsize> scratch{};
};

/* Simple handler for light brightness control */
static esp_err_t light_brightness_post_handler(httpd_req_t* req) {
    std::size_t total_len = req->content_len;
    int cur_len = 0;
    char* buf = ((RestServerContext*)(req->user_ctx))->scratch.data();
    int received = 0;
    if (total_len >= scratch_bufsize) {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "content too long");
        return ESP_FAIL;
    }
    while (cur_len < total_len) {
        received = httpd_req_recv(req, buf + cur_len, total_len);
        if (received <= 0) {
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                                "Failed to post control value");
            return ESP_FAIL;
        }
        cur_len += received;
    }
    buf[total_len] = '\0';

    cJSON* root = cJSON_Parse(buf);
    int red = cJSON_GetObjectItem(root, "red")->valueint;
    int green = cJSON_GetObjectItem(root, "green")->valueint;
    int blue = cJSON_GetObjectItem(root, "blue")->valueint;
    ESP_LOGI(rest_tag, "Light control: red = %d, green = %d, blue = %d", red,
             green, blue);
    cJSON_Delete(root);
    httpd_resp_sendstr(req, "Post control value successfully");
    return ESP_OK;
}

/* Simple handler for getting system handler */
static esp_err_t system_info_get_handler(httpd_req_t* req) {
    httpd_resp_set_type(req, "application/json");
    cJSON* root = cJSON_CreateObject();
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    cJSON_AddStringToObject(root, "version", IDF_VER);
    cJSON_AddNumberToObject(root, "cores", chip_info.cores);
    const char* sys_info = cJSON_Print(root);
    httpd_resp_sendstr(req, sys_info);
    free((void*)sys_info);
    cJSON_Delete(root);
    return ESP_OK;
}

/* Simple handler for getting temperature data */
static esp_err_t temperature_data_get_handler(httpd_req_t* req) {
    httpd_resp_set_type(req, "application/json");
    cJSON* root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "raw", esp_random() % 20);
    const char* sys_info = cJSON_Print(root);
    httpd_resp_sendstr(req, sys_info);
    free((void*)sys_info);
    cJSON_Delete(root);
    return ESP_OK;
}

namespace plant {

esp_err_t start_http_service() {
    auto* rest_context = new RestServerContext;
    REST_CHECK(rest_context, "No memory for rest context",
               [] { return ESP_FAIL; });

    httpd_handle_t server = nullptr;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(rest_tag, "Starting HTTP Server");
    REST_CHECK(httpd_start(&server, &config) == ESP_OK, "Start server failed",
               [&] {
                   delete rest_context;
                   return ESP_FAIL;
               });

    /* URI handler for fetching system info */
    httpd_uri_t system_info_get_uri = {.uri = "/api/v1/system/info",
                                       .method = HTTP_GET,
                                       .handler = system_info_get_handler,
                                       .user_ctx = rest_context};
    httpd_register_uri_handler(server, &system_info_get_uri);

    /* URI handler for fetching temperature data */
    httpd_uri_t temperature_data_get_uri = {.uri = "/api/v1/temp/raw",
                                            .method = HTTP_GET,
                                            .handler =
                                                temperature_data_get_handler,
                                            .user_ctx = rest_context};
    httpd_register_uri_handler(server, &temperature_data_get_uri);

    /* URI handler for light brightness control */
    httpd_uri_t light_brightness_post_uri = {.uri = "/api/v1/light/brightness",
                                             .method = HTTP_POST,
                                             .handler =
                                                 light_brightness_post_handler,
                                             .user_ctx = rest_context};
    httpd_register_uri_handler(server, &light_brightness_post_uri);

    return ESP_OK;
}

} // namespace plant