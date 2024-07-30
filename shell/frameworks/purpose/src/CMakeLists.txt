set(PURPOSE_INSTALL_INCLUDEDIR "${KDE_INSTALL_INCLUDEDIR_KF}/Purpose")

qt_add_resources(purpose_SRCS standard-plugin-types.qrc)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake-paths.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/cmake-paths.h)

ecm_qt_declare_logging_category(external_process_debug_SRCS
    HEADER purpose_external_process_debug.h
    IDENTIFIER PURPOSE_EXTERNAL_PROCESS_LOG
    CATEGORY_NAME kf.purpose.externalprocess
    OLD_CATEGORY_NAMES kf5.purpose.externalprocess
    DESCRIPTION "purpose external process"
    EXPORT PURPOSE
)

add_library(KF6Purpose)
add_library(KF6::Purpose ALIAS KF6Purpose)

qt_extract_metatypes(KF6Purpose)

set_target_properties(KF6Purpose PROPERTIES
    VERSION     ${PURPOSE_VERSION}
    SOVERSION   ${PURPOSE_SOVERSION}
    EXPORT_NAME Purpose
)

target_sources(KF6Purpose PRIVATE
    ${external_process_debug_SRCS}
    pluginbase.cpp
    job.cpp
    alternativesmodel.cpp
    configuration.cpp
    helper.cpp
    externalprocess/processjob.cpp
    jobcontroller.cpp
    ${purpose_SRCS}
)

target_include_directories(KF6Purpose
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
        "$<BUILD_INTERFACE:${Purpose_BINARY_DIR}>"
    INTERFACE
        "$<INSTALL_INTERFACE:${PURPOSE_INSTALL_INCLUDEDIR}>"
)
target_link_libraries(KF6Purpose
    PUBLIC
        KF6::CoreAddons
        Qt6::Gui
    PRIVATE
        Qt6::Network #QLocalSocket
        KF6::ConfigCore
)

if (HAVE_DBUS)
    target_link_libraries(KF6Purpose
        PRIVATE
        Qt6::DBus
    )
    target_compile_definitions(KF6Purpose PRIVATE -DHAVE_QTDBUS=1)
else()
    target_compile_definitions(KF6Purpose PRIVATE -DHAVE_QTDBUS=0)
endif()

install(TARGETS KF6Purpose EXPORT KF6PurposeTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

ecm_generate_export_header(KF6Purpose
    EXPORT_FILE_NAME ${Purpose_BINARY_DIR}/purpose/purpose_export.h
    BASE_NAME Purpose
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
)

ecm_generate_headers(Purpose_CamelCase_HEADERS
    HEADER_NAMES
        PluginBase
        AlternativesModel
        Job
        Configuration
    REQUIRED_HEADERS Purpose_HEADERS
    PREFIX Purpose
)

install(
    FILES ${Purpose_CamelCase_HEADERS}
    DESTINATION ${PURPOSE_INSTALL_INCLUDEDIR}/Purpose # C++ namespace
    COMPONENT Devel
)
install(
    FILES
        ${Purpose_BINARY_DIR}/purpose/purpose_export.h
        ${Purpose_HEADERS}
    DESTINATION ${PURPOSE_INSTALL_INCLUDEDIR}/purpose # C++ namespace
    COMPONENT Devel
)

add_executable(purposeprocess externalprocess/purposeprocess_main.cpp helper.cpp ${external_process_debug_SRCS})
ecm_mark_nongui_executable(purposeprocess)
target_link_libraries(purposeprocess KF6::Purpose Qt6::Network Qt6::Widgets)
install(TARGETS purposeprocess DESTINATION ${KDE_INSTALL_LIBEXECDIR_KF})

add_subdirectory(plugins)
add_subdirectory(quick)
add_subdirectory(widgets)
add_subdirectory(fileitemactionplugin)

ecm_qt_install_logging_categories(
    EXPORT PURPOSE
    FILE purpose.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
