# The part of the toolkit that needs neither a display nor Skia, for
# services and daemons as well as applications.
set(LINGMO_TOOLKIT_BASE_PUBLIC_HEADERS
    application_registry.h
    auto_reset.h
    byte_stream.h
    collator.h
    command_line.h
    compat.h
    dbus.h
    dbus_types.h
    elapsed_timer.h
    desktop_entry.h
    event_loop.h
    file.h
    file_watcher.h
    geometry.h
    hash.h
    i18n.h
    json.h
    local_socket.h
    locale.h
    logging.h
    lru_cache.h
    mime_apps.h
    mime_database.h
    paths.h
    process.h
    region.h
    session_language.h
    settings.h
    signal.h
    strings.h
    thread_pool.h
    timer.h
    transform.h
    url.h
    uuid.h
    weak_ptr.h
)

add_library(lingmo-sdk-base SHARED
    application_registry.cpp
    collator.cpp
    command_line.cpp
    dbus.cpp
    desktop_entry.cpp
    event_loop.cpp
    file.cpp
    file_watcher.cpp
    geometry.cpp
    hash.cpp
    i18n.cpp
    json.cpp
    local_socket.cpp
    locale.cpp
    logging.cpp
    mime_apps.cpp
    mime_database.cpp
    mime_detection.cpp
    paths.cpp
    process.cpp
    region.cpp
    session_language.cpp
    settings.cpp
    strings.cpp
    thread_pool.cpp
    timer.cpp
    transform.cpp
    url.cpp
    uuid.cpp
)

add_library(Lingmo::SDKBase ALIAS lingmo-sdk-base)

set_target_properties(lingmo-sdk-base PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    EXPORT_NAME SDKBase
)

target_include_directories(lingmo-sdk-base
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/lingmo-sdk>
)

# The desktop's sources spell the toolkit's namespaces the way the Cutefish
# toolkit did; compat.h aliases them. Force-included, it comes before the
# headers that use the names, in every translation unit that links the
# toolkit.
target_compile_options(lingmo-sdk-base
    INTERFACE
        $<BUILD_INTERFACE:-include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/compat.h>
        $<INSTALL_INTERFACE:-include>
        $<INSTALL_INTERFACE:base/compat.h>
)

target_compile_definitions(lingmo-sdk-base PRIVATE
    LINGMO_LOCALE_DIR="${CMAKE_INSTALL_FULL_LOCALEDIR}"
)

target_link_libraries(lingmo-sdk-base
    PRIVATE
        PkgConfig::Icu
        PkgConfig::Systemd
        Threads::Threads
)

install(TARGETS lingmo-sdk-base EXPORT LingmoSDKTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES ${LINGMO_TOOLKIT_BASE_PUBLIC_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lingmo-sdk/base
)
