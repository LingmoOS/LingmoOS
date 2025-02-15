add_library(KF5ConfigQml)
add_library(KF5::ConfigQml ALIAS KF5ConfigQml)

target_sources(KF5ConfigQml PRIVATE
    kconfigpropertymap.cpp
)

ecm_generate_export_header(KF5ConfigQml
    BASE_NAME KConfigQml
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    DEPRECATED_BASE_VERSION 0
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_link_libraries(KF5ConfigQml
  PUBLIC
    KF5::ConfigCore # KCoreConfigSkeleton, in ConfigPropertyMap
    Qt${QT_MAJOR_VERSION}::Qml
)
set_target_properties(KF5ConfigQml PROPERTIES VERSION     ${KCONFIG_VERSION}
                                              SOVERSION   ${KCONFIG_SOVERSION}
                                              EXPORT_NAME ConfigQml
)

ecm_generate_headers(KConfigQml_HEADERS
    HEADER_NAMES
    KConfigPropertyMap

    REQUIRED_HEADERS KConfigQml_HEADERS
)
target_include_directories(KF5ConfigQml
    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KConfig;${KDE_INSTALL_INCLUDEDIR_KF}/KConfigQml>")

install(TARGETS KF5ConfigQml EXPORT KF5ConfigTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kconfigqml_export.h
  ${KConfigQml_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KConfigQml COMPONENT Devel
)
