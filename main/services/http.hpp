#pragma once

#include <esp_err.h>

namespace plant {

[[nodiscard]] esp_err_t start_http_service();

} // namespace plant