set(PURPOSEWIDGETS_INSTALL_INCLUDEDIR "${KDE_INSTALL_INCLUDEDIR_KF}/PurposeWidgets")

add_library(KF6PurposeWidgets)
add_library(KF6::PurposeWidgets ALIAS KF6PurposeWidgets)

set_target_properties(KF6PurposeWidgets PROPERTIES
    VERSION     ${PURPOSE_VERSION}
    SOVERSION   ${PURPOSE_SOVERSION}
    EXPORT_NAME PurposeWidgets
)

qt_add_resources(qmlfiles_SRCS purposewidgets-resources.qrc)

target_sources(KF6PurposeWidgets PRIVATE
    menu.cpp
    ${qmlfiles_SRCS}
)

ecm_generate_export_header(KF6PurposeWidgets
    BASE_NAME PurposeWidgets
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    VERSION_BASE_NAME Purpose
    DEPRECATED_BASE_VERSION 0
)

target_link_libraries(KF6PurposeWidgets
    PUBLIC
        KF6::Purpose
        Qt6::Widgets
    PRIVATE
        Qt6::Qml
        KF6::I18n
)

target_include_directories(KF6PurposeWidgets
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
        "$<BUILD_INTERFACE:${Purpose_BINARY_DIR}>"
    INTERFACE
        "$<INSTALL_INTERFACE:${PURPOSEWIDGETS_INSTALL_INCLUDEDIR}>"
)

install(TARGETS KF6PurposeWidgets EXPORT KF6PurposeTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

ecm_generate_headers(PurposeWidgets_CamelCase_HEADERS
    HEADER_NAMES
        Menu
    REQUIRED_HEADERS PurposeWidgets_HEADERS
    PREFIX Purpose
)
install(
    FILES
        ${PurposeWidgets_CamelCase_HEADERS}
    DESTINATION ${PURPOSEWIDGETS_INSTALL_INCLUDEDIR}/Purpose # C++ namespace
    COMPONENT Devel
)
install(
    FILES
        ${Purpose_BINARY_DIR}/src/widgets/purposewidgets_export.h
        ${PurposeWidgets_HEADERS}
    DESTINATION ${PURPOSEWIDGETS_INSTALL_INCLUDEDIR}/purpose # C++ namespace
    COMPONENT Devel
)
