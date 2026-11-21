lingmo_add_client(bluez TARGET BluezClient
    SOURCES
        bluetooth_object.cpp
        devices_model.cpp
        manager.cpp
    HEADERS
        bluetooth_object.h
        devices_model.h
        manager.h
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
