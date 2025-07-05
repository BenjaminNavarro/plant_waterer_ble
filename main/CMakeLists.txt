idf_component_register(
    SRCS
        main.cpp
        data/watering_schedule.cpp
        # services/http.cpp
        services/ble_service.cpp
        services/ble_current_time_service.cpp
        services/ble_battery_service.cpp
        services/ble_watering_service.cpp
        tasks/blink.cpp
        tasks/watering.cpp
        tasks/hardware.cpp
        main_ble/main.cpp
        main_ble/src/gap.cpp
        main_ble/src/gatt_svc.cpp
        main_ble/src/led.cpp
    INCLUDE_DIRS
        .
        pid-utils/hashed-string
        pid-utils/unreachable
        main_ble/include
)

target_compile_definitions(
    __idf_main
    PRIVATE
        PID_UTILS_CXX17_AVAILABLE=1
)
