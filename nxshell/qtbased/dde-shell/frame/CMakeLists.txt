# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0
set(PUBLIC_HEADERS
    dsglobal.h
    dstypes.h
    pluginmetadata.h
    pluginloader.h
    pluginfactory.h
    applet.h
    appletdata.h
    containment.h
    panel.h
    qmlengine.h
    layershell/dlayershellwindow.h
    dsutility.h
)

set(PRIVATE_HEADERS
    private/applet_p.h
    private/containment_p.h
    private/panel_p.h
    private/appletitem_p.h
    private/dsqmlglobal_p.h
    layershell/qwaylandlayershellsurface_p.h
    layershell/qwaylandlayershellintegration_p.h
    quick/dsquickdrag_p.h
)
set(PRIVATE_SOURCES "")
if (BUILD_WITH_X11)
    list(APPEND PRIVATE_HEADERS
        private/utility_x11_p.h
    )
    list(APPEND PRIVATE_SOURCES
        utility_x11.cpp
    )
endif(BUILD_WITH_X11)

add_library(dde-shell-frame SHARED
    ${PUBLIC_HEADERS}
    ${PRIVATE_HEADERS}
    ${PRIVATE_SOURCES}
    appletitem.h
    containmentitem.h
    dstypes.cpp
    appletdata.cpp
    pluginmetadata.cpp
    pluginloader.cpp
    pluginfactory.cpp
    applet.cpp
    containment.cpp
    panel.cpp
    appletitem.cpp
    containmentitem.cpp
    qmlengine.cpp
    appletitemmodel.h
    appletitemmodel.cpp
    dsqmlglobal.cpp
    layershell/dlayershellwindow.cpp
    layershell/qwaylandlayershellsurface.cpp
    layershell/qwaylandlayershellintegration.cpp
    quick/dsquickdrag.cpp
    ddeshell_qml.qrc
    dsutility.cpp
)

qt_generate_wayland_protocol_client_sources(dde-shell-frame FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/layershell/protocol/wlr-layer-shell-unstable-v1.xml
    ${WaylandProtocols_DATADIR}/stable/xdg-shell/xdg-shell.xml
)

set_target_properties(dde-shell-frame PROPERTIES
    VERSION ${CMAKE_PROJECT_VERSION}
    SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR}
    OUTPUT_NAME dde-shell
    EXPORT_NAME Shell
)

target_link_libraries(dde-shell-frame
PUBLIC
    Dtk${DTK_VERSION_MAJOR}::Core
    Dtk${DTK_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Quick
PRIVATE
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::GuiPrivate
    Qt${QT_VERSION_MAJOR}::WaylandClientPrivate
)
target_include_directories(dde-shell-frame INTERFACE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
    $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>
)
target_link_directories(dde-shell-frame INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:${LIB_INSTALL_DIR}>
)

if (BUILD_WITH_X11)
    target_compile_definitions(dde-shell-frame PRIVATE BUILD_WITH_X11)
    pkg_check_modules(XCB REQUIRED IMPORTED_TARGET xcb-ewmh xtst x11)
    target_sources(dde-shell-frame PRIVATE layershell/x11dlayershellemulation.h layershell/x11dlayershellemulation.cpp)
    target_link_libraries(dde-shell-frame PRIVATE PkgConfig::XCB)
endif(BUILD_WITH_X11)

target_compile_definitions(dde-shell-frame
INTERFACE
    DDE_SHELL_PACKAGE_PATH_SUBPATH="dde-shell"
PRIVATE
    DDE_SHELL_QML_INSTALL_DIR="${CMAKE_INSTALL_PREFIX}/${QML_INSTALL_DIR}"
    DDE_SHELL_PLUGIN_INSTALL_DIR="${CMAKE_INSTALL_PREFIX}/${DDE_SHELL_PLUGIN_INSTALL_DIR}"
    DS_LIB
)

install(FILES ${PUBLIC_HEADERS} DESTINATION "${INCLUDE_INSTALL_DIR}")
install(TARGETS dde-shell-frame EXPORT DDEShellTargets DESTINATION "${LIB_INSTALL_DIR}")
install(EXPORT DDEShellTargets NAMESPACE Dde:: FILE DDEShellTargets.cmake DESTINATION "${CONFIG_INSTALL_DIR}")

add_subdirectory(plugin)
