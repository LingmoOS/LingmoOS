# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>

add_library(KF6ScreenDpms SHARED)
target_sources(KF6ScreenDpms PRIVATE dpms.cpp abstractdpmshelper.cpp waylanddpmshelper.cpp xcbdpmshelper.cpp waylanddpmshelper.cpp)
target_link_libraries(KF6ScreenDpms PUBLIC Qt::Gui
                                    PRIVATE XCB::XCB XCB::DPMS XCB::RANDR
                                            Qt::GuiPrivate Qt::WaylandClient Wayland::Client
)

qt6_generate_wayland_protocol_client_sources(KF6ScreenDpms FILES ${PLASMA_WAYLAND_PROTOCOLS_DIR}/dpms.xml)

set_target_properties(KF6ScreenDpms PROPERTIES
    VERSION "${KSCREEN_VERSION}"
    SOVERSION "${KSCREEN_SOVERSION}"
    EXPORT_NAME ScreenDpms
)
generate_export_header(KF6ScreenDpms BASE_NAME KScreenDpms)

install(TARGETS KF6ScreenDpms EXPORT KF6ScreenTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
add_library(KF6::ScreenDpms ALIAS KF6ScreenDpms)

ecm_qt_declare_logging_category(KF6ScreenDpms
                                HEADER kscreendpms_debug.h
                                IDENTIFIER KSCREEN_DPMS
                                CATEGORY_NAME org.kde.kscreen.dpms
)

ecm_generate_headers(KScreenDpms_HEADERS
    HEADER_NAMES
        Dpms
    PREFIX KScreenDpms
    REQUIRED_HEADERS KScreenDpms_REQ_HEADERS
)

target_include_directories(KF6ScreenDpms INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KScreen>"
                                                   "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}>"
)

install(FILES ${KScreenDpms_HEADERS}
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KScreen/KScreenDpms
        COMPONENT Devel)
install(FILES ${KScreenDpms_REQ_HEADERS} ${CMAKE_CURRENT_BINARY_DIR}/kscreendpms_export.h
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KScreen/kscreendpms)
