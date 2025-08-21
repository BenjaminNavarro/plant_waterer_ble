idf_component_register(
    SRCS
        main.cpp
        data/user_defined_name.cpp
        services/ble_service.cpp
        services/ble_characteristic.cpp
        services/ble_current_time_service.cpp
        services/ble_battery_service.cpp
        services/ble_name_service.cpp
        services/ble_watering_service/ble_watering_schedule.cpp
        services/ble_watering_service/ble_watering_test.cpp
        services/ble_watering_service/ble_watering_state.cpp
        services/ble_watering_service/ble_watering_service.cpp
        tasks/blink.cpp
        tasks/watering.cpp
        tasks/hardware.cpp
        tasks/schedule.cpp
        ble/gap.cpp
    INCLUDE_DIRS
        .
        pid-utils/hashed-string
        pid-utils/unreachable
)

target_compile_definitions(
    __idf_main
    PRIVATE
        PID_UTILS_CXX17_AVAILABLE=1
)

set_target_properties(
    __idf_main
    PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED TRUE
        CXX_EXTENSIONS FALSE
)