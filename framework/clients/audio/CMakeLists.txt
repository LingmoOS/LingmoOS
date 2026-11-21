find_package(PkgConfig REQUIRED)
pkg_check_modules(PIPEWIRE REQUIRED libpipewire-0.3>=0.3.65)
pkg_check_modules(SPA REQUIRED libspa-0.2)

lingmo_add_client(audio TARGET AudioClient
    SOURCES
        audio_model.cpp
        audio_object.cpp
        backend.cpp
        volume_monitor.cpp
    HEADERS
        audio_model.h
        audio_object.h
        backend.h
        volume_monitor.h
)
# By path and name rather than as imported targets, which the installed package would have to recreate.
target_include_directories(lingmo-audio-client PUBLIC ${PIPEWIRE_INCLUDE_DIRS} ${SPA_INCLUDE_DIRS})
target_link_libraries(lingmo-audio-client PUBLIC ${PIPEWIRE_LIBRARIES})

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
