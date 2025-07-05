#include <services/http.hpp>
#include <data/watering_schedule.hpp>
#include <tasks/watering.hpp>

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

namespace plant {

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
        auto get_int = [entry](const char* name) {
            return cJSON_GetObjectItem(entry, name)->valueint;
        };
        auto get_float = [entry](const char* name) {
            return cJSON_GetObjectItem(entry, name)->valuedouble;
        };

        group.enabled = get_int("enabled") == 1;
        group.start_time = get_int("start_time");
        group.watering_period = get_int("watering_period");
        group.watering_duration = get_int("watering_duration");
        group.flow_speed = FlowSpeed{get_float("flow_speed")};
    }
    cJSON_Delete(root);

    ESP_LOGI("schedule_post", "Request parsed");

    ESP_LOGI("schedule_post", "Saving schedule to storage");
    plant::write_schedule_to_storage(schedule);
    ESP_LOGI("schedule_post", "Schedule saved to storage");

    ESP_LOGI("schedule_post", "Posting schedule to queue...");
    xQueueOverwrite(ctx->params.watering_schedule_queue, &schedule);
    ESP_LOGI("schedule_post", "Schedule posted to queue");

    constexpr auto mode{plant::Mode::Automatic};
    xQueueSend(ctx->params.mode_switch_queue, &mode, portMAX_DELAY);

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
        auto set = [entry](const char* name, auto value) {
            cJSON_AddNumberToObject(entry, name, static_cast<double>(value));
        };

        set("enabled", group.enabled ? 1 : 0);
        set("start_time", group.start_time);
        set("watering_period", group.watering_period);
        set("watering_duration", group.watering_duration);
        set("flow_speed", group.flow_speed);

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
    config.flow_speed =
        plant::FlowSpeed{cJSON_GetObjectItem(root, "flow_speed")->valuedouble};
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

    constexpr auto mode{plant::Mode::Manual};
    xQueueSend(ctx->params.mode_switch_queue, &mode, portMAX_DELAY);

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
    cJSON_AddNumberToObject(root, "flow_speed", config.flow_speed.value());
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
    auto* mode_name = cJSON_GetObjectItem(root, "mode");

    const auto mode =
        mode_from_string(std::string_view{mode_name->valuestring});

    cJSON_Delete(root);

    ESP_LOGI("mode_post", "Request parsed");

    if (mode) {
        xQueueSend(ctx->params.mode_switch_queue, &mode, portMAX_DELAY);
        httpd_resp_sendstr(req, "ok");
    } else {
        httpd_resp_sendstr(req, "fail");
    }

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

esp_err_t program_test_post_handler(httpd_req_t* req) {
    add_cors_headers(req);

    ESP_LOGI("program_test_post", "Watering test request received");
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

    ESP_LOGI("program_test_post", "Parsing the request...");
    auto* root = cJSON_Parse(buf);
    config.output = cJSON_GetObjectItem(root, "output")->valueint;
    if (config.output > valve_count) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Output is out of range");
        return ESP_FAIL;
    }
    config.duration = cJSON_GetObjectItem(root, "duration")->valueint;
    config.flow_speed =
        plant::FlowSpeed{cJSON_GetObjectItem(root, "flow_speed")->valuedouble};

    cJSON_Delete(root);

    ESP_LOGI("program_test_post", "Request parsed");

    ESP_LOGI("program_test_post", "Posting watering test to queue...");
    xQueueSend(ctx->params.program_test_queue, &config, portMAX_DELAY);
    ESP_LOGI("program_test_post", "watering test posted to queue");

    constexpr auto mode{plant::Mode::WateringTest};
    xQueueSend(ctx->params.mode_switch_queue, &mode, portMAX_DELAY);

    httpd_resp_sendstr(req, "ok");

    return ESP_OK;
}

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

    httpd_uri_t test_get_uri = {.uri = "/api/v1/manual/read",
                                .method = HTTP_GET,
                                .handler = test_get_handler,
                                .user_ctx = rest_context};
    httpd_register_uri_handler(server, &test_get_uri);

    httpd_uri_t test_post_uri = {.uri = "/api/v1/manual/write",
                                 .method = HTTP_POST,
                                 .handler = test_post_handler,
                                 .user_ctx = rest_context};
    httpd_register_uri_handler(server, &test_post_uri);

    httpd_uri_t mode_post_uri = {.uri = "/api/v1/mode/write",
                                 .method = HTTP_POST,
                                 .handler = mode_post_handler,
                                 .user_ctx = rest_context};
    httpd_register_uri_handler(server, &mode_post_uri);

    httpd_uri_t program_test_post_uri = {.uri = "/api/v1/program_test/write",
                                         .method = HTTP_POST,
                                         .handler = program_test_post_handler,
                                         .user_ctx = rest_context};
    httpd_register_uri_handler(server, &program_test_post_uri);

    return ESP_OK;
}

} // namespace plant