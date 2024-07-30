# SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>
# SPDX-License-Identifier: BSD-2-Clause

add_library(KF6ConfigQml)
add_library(KF6::ConfigQml ALIAS KF6ConfigQml)

ecm_add_qml_module(KF6ConfigQml URI org.kde.config GENERATE_PLUGIN_SOURCE)

set_target_properties(KF6ConfigQml PROPERTIES
    VERSION     ${KCONFIG_VERSION}
    SOVERSION   ${KCONFIG_SOVERSION}
    EXPORT_NAME ConfigQml
)

target_sources(KF6ConfigQml PRIVATE
    kconfigpropertymap.cpp
    kwindowstatesaverquick.cpp
    types.cpp
)

ecm_qt_declare_logging_category(KF6ConfigQml
    HEADER kconfig_qml_log_settings.h
    IDENTIFIER KCONFIG_QML_LOG
    CATEGORY_NAME kf.config.qml
    DESCRIPTION "KConfig QML"
    EXPORT KCONFIG
)

ecm_generate_export_header(KF6ConfigQml
    BASE_NAME KConfigQml
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    VERSION_BASE_NAME KConfig
    DEPRECATED_BASE_VERSION 0
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_link_libraries(KF6ConfigQml
  PUBLIC
    KF6::ConfigCore # KCoreConfigSkeleton, in ConfigPropertyMap
    KF6::ConfigGui # KWindowStateSaver
    Qt6::Qml
  PRIVATE
    Qt6::Quick
)

ecm_generate_headers(KConfigQml_HEADERS
    HEADER_NAMES
    KConfigPropertyMap

    REQUIRED_HEADERS KConfigQml_HEADERS
)
target_include_directories(KF6ConfigQml
    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KConfig;${KDE_INSTALL_INCLUDEDIR_KF}/KConfigQml>")

install(TARGETS KF6ConfigQml EXPORT KF6ConfigTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kconfigqml_export.h
  ${KConfigQml_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KConfigQml COMPONENT Devel
)

ecm_finalize_qml_module(KF6ConfigQml DESTINATION ${KDE_INSTALL_QMLDIR})
