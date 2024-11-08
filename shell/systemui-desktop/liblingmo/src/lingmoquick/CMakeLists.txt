# Consumer's include dir which has to be explicitly used to make headers of this lib visible to documented includes
# Results in duplicate of prefix-dir & C++ namespace below, but part of different things, so by design:
# <visibility-layer>/<C++ namespace>/class header files
set(LINGMOQUICK_INSTALL_INCLUDEDIR "${KDE_INSTALL_INCLUDEDIR}/LingmoQuick")

add_library(LingmoQuick SHARED)
add_library(Lingmo::LingmoQuick ALIAS LingmoQuick)

qt_extract_metatypes(LingmoQuick)

set_target_properties(LingmoQuick PROPERTIES
    VERSION     ${LINGMO_VERSION}
    SOVERSION   ${LINGMO_SOVERSION}
    EXPORT_NAME LingmoQuick
)

target_sources(LingmoQuick PRIVATE
    appletcontext.cpp
    appletquickitem.cpp
    appletpopup.cpp
    debug_p.cpp
    dialog.cpp
    dialogshadows.cpp
    containmentview.cpp
    configmodel.cpp
    configview.cpp
    edgeeventforwarder.cpp
    lingmoshellwaylandintegration.cpp
    sharedqmlengine.cpp
    quickviewsharedengine.cpp
    lingmowindow.cpp
    popuplingmowindow.cpp
    transientplacementhint.cpp
    windowresizehandler.cpp
    configcategory_p.cpp
    plasmoidattached_p.cpp
    dialogbackground_p.cpp
    utils.cpp
    plasmoid/plasmoiditem.cpp
    plasmoid/containmentitem.cpp
    plasmoid/dropmenu.cpp
    plasmoid/wallpaperitem.cpp
)

qt_generate_wayland_protocol_client_sources(LingmoQuick
    FILES
        "${LINGMO_WAYLAND_PROTOCOLS_DIR}/lingmo-shell.xml"
        "${Wayland_DATADIR}/wayland.xml"
)

ecm_qt_declare_logging_category(LingmoQuick
    HEADER debug_p.h
    IDENTIFIER LOG_LINGMOQUICK
    CATEGORY_NAME kf.lingmo.quick
    OLD_CATEGORY_NAMES org.kde.lingmoquick
    DESCRIPTION "Lingmo Quick lib"
    EXPORT LINGMO
)

set_target_properties(LingmoQuick PROPERTIES
    VERSION     ${LINGMO_VERSION}
    SOVERSION   ${LINGMO_SOVERSION}
    EXPORT_NAME LingmoQuick
)

target_include_directories(LingmoQuick
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR};${CMAKE_CURRENT_BINARY_DIR}/..>"
    INTERFACE
        "$<INSTALL_INTERFACE:${LINGMOQUICK_INSTALL_INCLUDEDIR}>"
        "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/Lingmo>" # module version header
)

target_link_libraries(LingmoQuick
    PUBLIC
        Qt6::Gui
        Qt6::Quick
        Qt6::Qml
        Lingmo::Lingmo
        KF6::WindowSystem
    PRIVATE
        Qt6::Svg
        Qt6::GuiPrivate
        Qt6::WaylandClient
        Wayland::Client
        KF6::ConfigGui
        KF6::ConfigQml
        KF6::I18n
        KF6::IconThemes
        KF6::CoreAddons
        KF6::Package
        KF6::KIOCore
        KF6::KIOWidgets
        KF6::Notifications
        KF6::KCMUtilsQuick
        KF6::Svg
        KF6::GuiAddons
)

if(HAVE_X11)
    target_link_libraries(LingmoQuick
        PRIVATE
            ${X11_LIBRARIES}
            XCB::XCB
            Qt6::GuiPrivate
    )
endif()

install(TARGETS LingmoQuick EXPORT LingmoQuickTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

ecm_generate_export_header(LingmoQuick
    BASE_NAME LingmoQuick
    GROUP_BASE_NAME KF
    VERSION ${KF6_MIN_VERSION}
    USE_VERSION_HEADER
    VERSION_BASE_NAME Lingmo
    DEPRECATED_BASE_VERSION 0
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
    DEPRECATION_VERSIONS
)

set(lingmoquick_LIB_INCLUDES
    ${CMAKE_CURRENT_BINARY_DIR}/lingmoquick_export.h
)

ecm_generate_headers(LingmoQuick_CamelCase_HEADERS
    HEADER_NAMES
        AppletQuickItem
        ContainmentView
        ConfigView
        ConfigModel
        Dialog
        LingmoWindow
        LingmoShellWaylandIntegration
        PopupLingmoWindow
        AppletPopup
        SharedQmlEngine
        QuickViewSharedEngine
    REQUIRED_HEADERS lingmoquick_LIB_INCLUDES
    PREFIX LingmoQuick
)

install(
    FILES ${lingmoquick_LIB_INCLUDES}
    DESTINATION ${LINGMOQUICK_INSTALL_INCLUDEDIR}/lingmoquick
    COMPONENT Devel
)

install(
    FILES ${LingmoQuick_CamelCase_HEADERS}
    DESTINATION ${LINGMOQUICK_INSTALL_INCLUDEDIR}/LingmoQuick
    COMPONENT Devel
)

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/LingmoQuick")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/LingmoQuickConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/LingmoQuickConfig.cmake"
    INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
    PATH_VARS  LINGMOQUICK_INSTALL_INCLUDEDIR CMAKE_INSTALL_PREFIX
)

ecm_setup_version(${LINGMO_VERSION}
                  VARIABLE_PREFIX LINGMOQUICK
                  SOVERSION 6
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/LingmoQuickConfigVersion.cmake" )

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/LingmoQuickConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/LingmoQuickConfigVersion.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}" COMPONENT Devel
)

install(EXPORT LingmoQuickTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE LingmoQuickTargets.cmake NAMESPACE Lingmo:: )
