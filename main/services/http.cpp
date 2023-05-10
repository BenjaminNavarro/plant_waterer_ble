#include <services/http.hpp>
#include <data/watering_schedule.hpp>

#include <cJSON.h>
#include <esp_chip_info.h>
#include <esp_http_server.h>
#include <esp_log.h>
#include <esp_random.h>
#include <fcntl.h>

#include <pid/hashed_string.h>
#include <pid/unreachable.h>

#include <array>
#include <cstring>
#include <string_view>

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
    explicit RestServerContext(plant::HttpServiceParams srv_params)
        : params{srv_params} {
    }

    std::array<char, scratch_bufsize> scratch{};
    plant::HttpServiceParams params;

    enum class Mode { Automatic, Manual, WateringTest, Invalid };
    Mode mode{Mode::Automatic};
};

constexpr std::string_view to_string(RestServerContext::Mode mode) {
    switch (mode) {
    case RestServerContext::Mode::Automatic:
        return "automatic";
    case RestServerContext::Mode::Manual:
        return "manual";
    case RestServerContext::Mode::WateringTest:
        return "watering_test";
    case RestServerContext::Mode::Invalid:
        return "invalid";
    }
    pid::unreachable();
}

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
    xQueueOverwrite(ctx->params.watering_schedule_queue, &schedule);
    ESP_LOGI("schedule_post", "Schedule posted to queue");

    httpd_resp_sendstr(req, "ok");

    return ESP_OK;
}

esp_err_t schedule_get_handler(httpd_req_t* req) {
    ESP_LOGI("schedule_get", "Schedule consultation request received");

    const auto* ctx = reinterpret_cast<RestServerContext*>(req->user_ctx);
    plant::WateringSchedule schedule;
    xQueuePeek(ctx->params.watering_schedule_queue, schedule.data(),
               portMAX_DELAY);

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

esp_err_t test_post_handler(httpd_req_t* req) {
    add_cors_headers(req);

    ESP_LOGI("test_post", "Test config modification request received");
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

    plant::HardwareState config;

    ESP_LOGI("test_post", "Parsing the request...");
    auto* root = cJSON_Parse(buf);
    config.pump_state = cJSON_GetObjectItem(root, "pump_state")->valueint == 1;
    auto* output_state = cJSON_GetObjectItem(root, "output_state");
    if (cJSON_GetArraySize(output_state) != config.output_state.size()) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Invalid number of test outputs");
        return ESP_FAIL;
    }
    for (int i = 0; i < config.output_state.size(); i++) {
        const auto* entry = cJSON_GetArrayItem(output_state, i);
        config.output_state[i] = entry->valueint == 1;
    }

    cJSON_Delete(root);

    ESP_LOGI("test_post", "Request parsed");

    ESP_LOGI("test_post", "Posting test config to queue...");
    xQueueOverwrite(ctx->params.test_configuration_queue, &config);
    ESP_LOGI("test_post", "test config posted to queue");

    httpd_resp_sendstr(req, "ok");

    return ESP_OK;
}

esp_err_t test_get_handler(httpd_req_t* req) {
    ESP_LOGI("test_get", "Test config consultation request received");

    const auto* ctx = reinterpret_cast<RestServerContext*>(req->user_ctx);
    plant::HardwareState config;
    xQueuePeek(ctx->params.test_configuration_queue, &config, portMAX_DELAY);

    ESP_LOGI("test_get", "Generating the JSON representation");
    httpd_resp_set_type(req, "application/json");
    add_cors_headers(req);

    cJSON* root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "pump_state", config.pump_state ? 1 : 0);
    auto* output_state = cJSON_AddArrayToObject(root, "output_state");
    for (const auto& state : config.output_state) {
        auto* number = cJSON_CreateNumber(state ? 1 : 0);
        cJSON_AddItemToArray(output_state, number);
    }

    ESP_LOGI("test_get", "Convert the JSON to text");
    const char* config_str = cJSON_PrintUnformatted(root);
    ESP_LOGI("test_get", "Send the response");
    httpd_resp_sendstr(req, config_str);
    free((void*)config_str);
    cJSON_Delete(root);
    return ESP_OK;
}

esp_err_t mode_post_handler(httpd_req_t* req) {
    add_cors_headers(req);

    ESP_LOGI("mode_post", "Test config modification request received");
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

    ESP_LOGI("mode_post", "Parsing the request...");
    auto* root = cJSON_Parse(buf);
    auto* mode = cJSON_GetObjectItem(root, "mode");

    using namespace pid::literals;
    switch (pid::hashed_string(mode->valuestring)) {
    case "automatic"_hs:
        if (ctx->mode != RestServerContext::Mode::Automatic) {
            ESP_LOGI("mode_post", "Switching to automatic mode");

            // Clear the test queue to let the task exit its loop and suspend itself
            xQueueReset(ctx->params.test_configuration_queue);

            vTaskResume(ctx->params.watering_task);

            ctx->mode = RestServerContext::Mode::Automatic;
        }
        break;
    case "manual"_hs:
        if (ctx->mode != RestServerContext::Mode::Manual) {
            ESP_LOGI("mode_post", "Switching to manual mode");

            // Reset the test configuration
            plant::HardwareState config;
            xQueueOverwrite(ctx->params.test_configuration_queue, &config);

            vTaskSuspend(ctx->params.watering_task);
            vTaskResume(ctx->params.test_task);

            ctx->mode = RestServerContext::Mode::Manual;
        }
        break;
    default:
        ctx->mode = RestServerContext::Mode::Invalid;
        vTaskSuspend(ctx->params.watering_task);
        vTaskSuspend(ctx->params.test_task);
        break;
    }

    cJSON_Delete(root);

    ESP_LOGI("mode_post", "Request parsed");

    if (ctx->mode != RestServerContext::Mode::Invalid) {
        httpd_resp_sendstr(req, "ok");
    } else {
        httpd_resp_sendstr(req, "fail");
    }

    return ESP_OK;
}

esp_err_t mode_get_handler(httpd_req_t* req) {
    ESP_LOGI("mode_get", "Test config consultation request received");

    const auto* ctx = reinterpret_cast<RestServerContext*>(req->user_ctx);
    plant::HardwareState config;
    xQueuePeek(ctx->params.test_configuration_queue, &config, portMAX_DELAY);

    ESP_LOGI("mode_get", "Generating the JSON representation");
    httpd_resp_set_type(req, "application/json");
    add_cors_headers(req);

    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "mode", to_string(ctx->mode).data());

    ESP_LOGI("mode_get", "Convert the JSON to text");
    const char* config_str = cJSON_PrintUnformatted(root);
    ESP_LOGI("mode_get", "Send the response");
    httpd_resp_sendstr(req, config_str);
    free((void*)config_str);
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

esp_err_t watering_test_post_handler(httpd_req_t* req) {
    add_cors_headers(req);

    ESP_LOGI("watering_test_post", "Watering test request received");
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

    plant::WateringTest config;

    ESP_LOGI("watering_test_post", "Parsing the request...");
    auto* root = cJSON_Parse(buf);
    config.output = cJSON_GetObjectItem(root, "ouput")->valueint;
    if (config.output > 8) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Output is out of range");
        return ESP_FAIL;
    }
    config.duration = cJSON_GetObjectItem(root, "duration")->valueint;

    cJSON_Delete(root);

    ESP_LOGI("watering_test_post", "Request parsed");

    ESP_LOGI("watering_test_post", "Posting watering test to queue...");
    xQueueSend(ctx->params.watering_test_queue, &config, portMAX_DELAY);
    ESP_LOGI("watering_test_post", "watering test posted to queue");

    httpd_resp_sendstr(req, "ok");

    return ESP_OK;
}

namespace plant {

esp_err_t start_http_service(HttpServiceParams params) {
    auto* rest_context = new RestServerContext{params};
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

    httpd_uri_t test_get_uri = {.uri = "/api/v1/test/read",
                                .method = HTTP_GET,
                                .handler = test_get_handler,
                                .user_ctx = rest_context};
    httpd_register_uri_handler(server, &test_get_uri);

    httpd_uri_t test_post_uri = {.uri = "/api/v1/test/write",
                                 .method = HTTP_POST,
                                 .handler = test_post_handler,
                                 .user_ctx = rest_context};
    httpd_register_uri_handler(server, &test_post_uri);

    httpd_uri_t mode_get_uri = {.uri = "/api/v1/mode/read",
                                .method = HTTP_GET,
                                .handler = mode_get_handler,
                                .user_ctx = rest_context};
    httpd_register_uri_handler(server, &mode_get_uri);

    httpd_uri_t mode_post_uri = {.uri = "/api/v1/mode/write",
                                 .method = HTTP_POST,
                                 .handler = mode_post_handler,
                                 .user_ctx = rest_context};
    httpd_register_uri_handler(server, &mode_post_uri);

    httpd_uri_t watering_test_post_uri = {.uri = "/api/v1/watering_test/write",
                                          .method = HTTP_POST,
                                          .handler = watering_test_post_handler,
                                          .user_ctx = rest_context};
    httpd_register_uri_handler(server, &watering_test_post_uri);

    return ESP_OK;
}

} // namespace plant