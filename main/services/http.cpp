#include <services/http.hpp>
#include <data/watering_schedule.hpp>

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
    explicit RestServerContext(QueueHandle_t schedule)
        : watering_schedule{schedule} {
    }

    std::array<char, scratch_bufsize> scratch{};
    QueueHandle_t watering_schedule{};
};

void add_cors_headers(httpd_req_t* req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "*");
}

esp_err_t schedule_post_handler(httpd_req_t* req) {
    add_cors_headers(req);

    ESP_LOGI("schedule_post", "Schedule modification request received");
    std::size_t total_len = req->content_len;
    int cur_len = 0;
    auto* ctx = reinterpret_cast<RestServerContext*>(req->user_ctx);
    char* buf = ctx->scratch.data();
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

    plant::WateringSchedule schedule;

    ESP_LOGI("schedule_post", "Parsing the request...");
    auto* root = cJSON_Parse(buf);
    auto* schedule_json = cJSON_GetObjectItem(root, "schedule");
    if (cJSON_GetArraySize(schedule_json) != schedule.size()) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Invalid number of group schedules");
        return ESP_FAIL;
    }
    for (int i = 0; i < schedule.size(); i++) {
        const auto* entry = cJSON_GetArrayItem(schedule_json, i);
        auto& group = schedule[i];
        auto get = [entry](const char* name) {
            return cJSON_GetObjectItem(entry, name)->valueint;
        };

        group.enabled = get("enabled") == 1;
        group.start_time = get("start_time");
        group.watering_period = get("watering_period");
        group.watering_duration = get("watering_duration");
    }
    cJSON_Delete(root);

    ESP_LOGI("schedule_post", "Request parsed");

    ESP_LOGI("schedule_post", "Saving schedule to storage");
    plant::write_schedule_from_storage(schedule);
    ESP_LOGI("schedule_post", "Schedule saved to storage");

    ESP_LOGI("schedule_post", "Posting schedule to queue...");
    xQueueOverwrite(ctx->watering_schedule, &schedule);
    ESP_LOGI("schedule_post", "Schedule posted to queue");

    httpd_resp_sendstr(req, "ok");

    return ESP_OK;
}

esp_err_t schedule_get_handler(httpd_req_t* req) {
    ESP_LOGI("schedule_get", "Schedule consultation request received");

    const auto* ctx = reinterpret_cast<RestServerContext*>(req->user_ctx);
    plant::WateringSchedule schedule;
    xQueuePeek(ctx->watering_schedule, schedule.data(), portMAX_DELAY);

    ESP_LOGI("schedule_get", "Generating the JSON representation");
    httpd_resp_set_type(req, "application/json");
    add_cors_headers(req);

    cJSON* root = cJSON_CreateObject();
    auto* schedule_json = cJSON_AddArrayToObject(root, "schedule");
    for (const auto& group : schedule) {
        auto* entry = cJSON_CreateObject();
        auto set = [entry](const char* name, std::uint64_t value) {
            cJSON_AddNumberToObject(entry, name, static_cast<double>(value));
        };

        set("enabled", group.enabled ? 1 : 0);
        set("start_time", group.start_time);
        set("watering_period", group.watering_period);
        set("watering_duration", group.watering_duration);

        cJSON_AddItemToArray(schedule_json, entry);
    }

    ESP_LOGI("schedule_get", "Convert the JSON to text");
    const char* schedule_str = cJSON_PrintUnformatted(root);
    ESP_LOGI("schedule_get", "Send the response");
    httpd_resp_sendstr(req, schedule_str);
    free((void*)schedule_str);
    cJSON_Delete(root);
    return ESP_OK;
}

esp_err_t system_info_get_handler(httpd_req_t* req) {
    httpd_resp_set_type(req, "application/json");
    add_cors_headers(req);

    cJSON* root = cJSON_CreateObject();
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    cJSON_AddStringToObject(root, "version", IDF_VER);
    cJSON_AddNumberToObject(root, "cores", chip_info.cores);
    const char* sys_info = cJSON_PrintUnformatted(root);
    httpd_resp_sendstr(req, sys_info);
    free((void*)sys_info);
    cJSON_Delete(root);
    return ESP_OK;
}

namespace plant {

esp_err_t start_http_service(QueueHandle_t watering_schedule) {
    auto* rest_context = new RestServerContext{watering_schedule};
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

    httpd_uri_t system_info_get_uri = {.uri = "/api/v1/system/info",
                                       .method = HTTP_GET,
                                       .handler = system_info_get_handler,
                                       .user_ctx = rest_context};
    httpd_register_uri_handler(server, &system_info_get_uri);

    httpd_uri_t schedule_get_uri = {.uri = "/api/v1/schedule/read",
                                    .method = HTTP_GET,
                                    .handler = schedule_get_handler,
                                    .user_ctx = rest_context};
    httpd_register_uri_handler(server, &schedule_get_uri);

    httpd_uri_t schedule_post_uri = {.uri = "/api/v1/schedule/write",
                                     .method = HTTP_POST,
                                     .handler = schedule_post_handler,
                                     .user_ctx = rest_context};
    httpd_register_uri_handler(server, &schedule_post_uri);

    return ESP_OK;
}

} // namespace plant