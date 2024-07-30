# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6Style")

add_library(KF6Style kstyle.cpp)
add_library(KF6::Style ALIAS KF6Style)

set_target_properties(KF6Style PROPERTIES
    VERSION     ${FRAMEWORKINTEGRATION_VERSION}
    SOVERSION   ${FRAMEWORKINTEGRATION_SOVERSION}
    EXPORT_NAME Style
)

ecm_generate_export_header(KF6Style
    BASE_NAME KStyle
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    VERSION_BASE_NAME FrameworkIntegration
    DEPRECATED_BASE_VERSION 0
)

target_include_directories(KF6Style
    INTERFACE
        "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/FrameworkIntegration>" # version header
        "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KStyle>"
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>" # version header
)

target_link_libraries(KF6Style
    PUBLIC
        Qt6::Widgets
    PRIVATE
        KF6::WidgetsAddons
        KF6::ColorScheme
        KF6::IconThemes
)

ecm_generate_headers(KStyle_HEADERS
  HEADER_NAMES
    KStyle
  REQUIRED_HEADERS KStyle_HEADERS
)

install(TARGETS KF6Style
        EXPORT KF6FrameworkIntegrationTargets
        ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kstyle_export.h
              ${KStyle_HEADERS}
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KStyle
        COMPONENT Devel)

