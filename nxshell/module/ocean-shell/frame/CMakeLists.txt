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
    appletproxy.h
    appletbridge.h
    qmlengine.h
    layershell/dlayershellwindow.h
    models/listtotableproxymodel.h
    dsutility.h
)

set(PRIVATE_HEADERS
    private/applet_p.h
    private/containment_p.h
    private/panel_p.h
    private/appletitem_p.h
    private/appletproxy_p.h
    private/appletbridge_p.h
    private/dsqmlglobal_p.h
    layershell/qwaylandlayershellsurface_p.h
    layershell/qwaylandlayershellintegration_p.h
    models/kextracolumnsproxymodel.h
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

add_library(ocean-shell-frame SHARED
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
    appletproxy.cpp
    appletbridge.cpp
    appletitem.cpp
    containmentitem.cpp
    qmlengine.cpp
    appletitemmodel.h
    appletitemmodel.cpp
    dsqmlglobal.cpp
    layershell/dlayershellwindow.cpp
    layershell/qwaylandlayershellsurface.cpp
    layershell/qwaylandlayershellintegration.cpp
    models/kextracolumnsproxymodel.cpp
    models/listtotableproxymodel.cpp
    quick/dsquickdrag.cpp
    popupwindow.h
    popupwindow.cpp
    oceanshell_qml.qrc
    dsutility.cpp
)

set_target_properties(ocean-shell-frame PROPERTIES
    PUBLIC_HEADER "${PUBLIC_HEADERS}"
)

qt_generate_wayland_protocol_client_sources(ocean-shell-frame FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/layershell/protocol/wlr-layer-shell-unstable-v1.xml
    ${WaylandProtocols_DATADIR}/stable/xdg-shell/xdg-shell.xml
)

set_target_properties(ocean-shell-frame PROPERTIES
    VERSION ${CMAKE_PROJECT_VERSION}
    SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR}
    OUTPUT_NAME ocean-shell
    EXPORT_NAME Shell
)

target_link_libraries(ocean-shell-frame
PUBLIC
    Dtk${DTK_VERSION_MAJOR}::Core
    Dtk${DTK_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Quick
PRIVATE
    Qt${QT_VERSION_MAJOR}::QuickPrivate
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::GuiPrivate
    Qt${QT_VERSION_MAJOR}::WaylandClientPrivate
)
target_include_directories(ocean-shell-frame INTERFACE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
    $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>
)
target_link_directories(ocean-shell-frame INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:${LIB_INSTALL_DIR}>
)

if (BUILD_WITH_X11)
    target_compile_definitions(ocean-shell-frame PRIVATE BUILD_WITH_X11)
    pkg_check_modules(XCB REQUIRED IMPORTED_TARGET xcb-ewmh xtst x11)
    target_sources(ocean-shell-frame PRIVATE layershell/x11dlayershellemulation.h layershell/x11dlayershellemulation.cpp)
    target_link_libraries(ocean-shell-frame PRIVATE PkgConfig::XCB)
endif(BUILD_WITH_X11)

target_compile_definitions(ocean-shell-frame
INTERFACE
    OCEAN_SHELL_PACKAGE_PATH_SUBPATH="ocean-shell"
PRIVATE
    OCEAN_SHELL_QML_INSTALL_DIR="${CMAKE_INSTALL_PREFIX}/${QML_INSTALL_DIR}"
    OCEAN_SHELL_PLUGIN_INSTALL_DIR="${CMAKE_INSTALL_PREFIX}/${OCEAN_SHELL_PLUGIN_INSTALL_DIR}"
    DS_LIB
)

install(TARGETS ocean-shell-frame EXPORT OCEANShellTargets DESTINATION "${LIB_INSTALL_DIR}" PUBLIC_HEADER DESTINATION "${INCLUDE_INSTALL_DIR}")
install(EXPORT OCEANShellTargets NAMESPACE Dde:: FILE OCEANShellTargets.cmake DESTINATION "${CONFIG_INSTALL_DIR}")

add_subdirectory(plugin)
