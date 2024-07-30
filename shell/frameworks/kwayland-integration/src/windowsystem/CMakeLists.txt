add_library(KF5WindowSystemKWaylandPlugin MODULE)
set(wayland_plugin_SRCS
    plugin.cpp
    shm.cpp
    waylandintegration.cpp
    windoweffects.cpp
    windowinfo.cpp
    windowshadow.cpp
    windowsystem.cpp
    waylandxdgactivationv1.cpp
    plugin.h
    waylandintegration.h
    windoweffects.h
    windowinfo.h
    windowshadow.h
    windowsystem.h
    waylandxdgactivationv1_p.h
)
ecm_add_qtwayland_client_protocol(wayland_plugin_SRCS
    PROTOCOL ${WaylandProtocols_DATADIR}/staging/xdg-activation/xdg-activation-v1.xml
    BASENAME xdg-activation-v1
)
ecm_add_qtwayland_client_protocol(wayland_plugin_SRCS
    PROTOCOL ${PLASMA_WAYLAND_PROTOCOLS_DIR}/blur.xml
    BASENAME blur
)
ecm_add_qtwayland_client_protocol(wayland_plugin_SRCS
    PROTOCOL ${PLASMA_WAYLAND_PROTOCOLS_DIR}/contrast.xml
    BASENAME contrast
)
ecm_add_qtwayland_client_protocol(wayland_plugin_SRCS
    PROTOCOL ${PLASMA_WAYLAND_PROTOCOLS_DIR}/slide.xml
    BASENAME slide
)
ecm_add_qtwayland_client_protocol(wayland_plugin_SRCS
    PROTOCOL ${PLASMA_WAYLAND_PROTOCOLS_DIR}/plasma-window-management.xml
    BASENAME plasma-window-management
)
ecm_add_qtwayland_client_protocol(wayland_plugin_SRCS
    PROTOCOL ${PLASMA_WAYLAND_PROTOCOLS_DIR}/shadow.xml
    BASENAME shadow
)
ecm_add_qtwayland_client_protocol(wayland_plugin_SRCS
    PROTOCOL ${Wayland_DATADIR}/wayland.xml
    BASENAME wayland
)

ecm_qt_declare_logging_category(wayland_plugin_SRCS HEADER logging.h IDENTIFIER KWAYLAND_KWS
    CATEGORY_NAME org.kde.kf5.kwindowsystem.kwayland DEFAULT_SEVERITY Warning
    DESCRIPTION "wayland integration (windowsystem)" EXPORT KWAYLAND)

target_sources(KF5WindowSystemKWaylandPlugin PRIVATE ${wayland_plugin_SRCS})

if(HAVE_MEMFD)
    target_compile_definitions(KF5WindowSystemKWaylandPlugin PRIVATE -DHAVE_MEMFD)
endif()

pkg_check_modules(XKBCommon REQUIRED IMPORTED_TARGET xkbcommon)

target_link_libraries(KF5WindowSystemKWaylandPlugin
    KF5::WindowSystem
    Qt::Widgets
    Wayland::Client
    Qt::WaylandClient
    Qt::WaylandClientPrivate
    PkgConfig::XKBCommon
)

target_link_libraries(KF5WindowSystemKWaylandPlugin
    Qt::XkbCommonSupportPrivate
    KF5::WaylandClient
)

install(
    TARGETS
        KF5WindowSystemKWaylandPlugin
    DESTINATION
        ${KDE_INSTALL_PLUGINDIR}/kf5/kwindowsystem/
)

ecm_qt_install_logging_categories(
        EXPORT KWAYLAND
        FILE kwindowsystem.kwayland.categories
        DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
        )

