lingmo_add_client(display TARGET DisplayClient
    SOURCES
        display_client.cpp
        night_light.cpp
        output_model.cpp
        screen.cpp
    HEADERS
        display_client.h
        display_types.h
        night_light.h
        output_model.h
        screen.h
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
