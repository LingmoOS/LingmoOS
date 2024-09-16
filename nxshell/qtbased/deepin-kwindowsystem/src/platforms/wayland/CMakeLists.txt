set(wayland_plugin_SRCS
    kwindowsystem.cpp
    plugin.cpp
)

add_library(KF5WindowSystemWaylandPlugin MODULE ${wayland_plugin_SRCS})
target_link_libraries(KF5WindowSystemWaylandPlugin
    KF5WindowSystem
)

set_target_properties(
    KF5WindowSystemWaylandPlugin
    PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/kf5/org.kde.kwindowsystem.platforms"
)

install(
    TARGETS
        KF5WindowSystemWaylandPlugin
    DESTINATION
        ${PLUGIN_INSTALL_DIR}/kf5/org.kde.kwindowsystem.platforms/
)
