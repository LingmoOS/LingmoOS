include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(LIBKBOLT_SRCS
    dbushelper.cpp
    device.cpp
    devicemodel.cpp
    enum.cpp
    manager.cpp
)

ecm_qt_declare_logging_category(LIBKBOLT_SRCS
    HEADER libkbolt_debug.h
    IDENTIFIER log_libkbolt
    CATEGORY_NAME org.kde.libkbolt
)

qt_add_dbus_interfaces(
    LIBKBOLT_SRCS

    ../interfaces/org.freedesktop.bolt1.manager.xml
    ../interfaces/org.freedesktop.bolt1.device.xml
)

add_library(kbolt SHARED ${LIBKBOLT_SRCS})
set_target_properties(kbolt PROPERTIES CXX_STANDARD 14)
generate_export_header(kbolt)
target_link_libraries(kbolt
    Qt6::Core
    Qt6::DBus
    KF6::I18n
)

target_include_directories(kbolt PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

# Don't create soname for kbolt, it's a private library
install(TARGETS kbolt ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
