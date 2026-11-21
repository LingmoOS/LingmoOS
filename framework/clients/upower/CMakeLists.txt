lingmo_add_client(upower TARGET UPowerClient
    SOURCES battery_status.cpp device.cpp
    HEADERS battery_status.h device.h
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
