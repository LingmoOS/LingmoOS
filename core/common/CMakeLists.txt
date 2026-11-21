# What the session, the runtime and the services share: the locale and proxy
# variables every application is started with, the GSettings keyfile and the
# runtime's client, and calls whose pending reply nobody keeps.
pkg_check_modules(ICU REQUIRED IMPORTED_TARGET icu-uc)
pkg_check_modules(CommonSystemd REQUIRED IMPORTED_TARGET libsystemd)

add_library(lingmo-core-common STATIC
    detached_call.cpp
    detached_call.h
    detached_process.cpp
    detached_process.h
    environment.cpp
    environment.h
    gsettings_keyfile.cpp
    gsettings_keyfile.h
    language_environment.cpp
    language_environment.h
    proxy_settings.cpp
    proxy_settings.h
    runtime_client.cpp
    runtime_client.h
)
target_include_directories(lingmo-core-common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(lingmo-core-common
    PUBLIC Lingmo::SDKBase
    PRIVATE PkgConfig::ICU PkgConfig::CommonSystemd
)
