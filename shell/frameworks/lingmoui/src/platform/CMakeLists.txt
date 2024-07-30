add_library(LingmoUIPlatform)
add_library(KF6::LingmoUIPlatform ALIAS LingmoUIPlatform)

ecm_add_qml_module(LingmoUIPlatform URI "org.kde.lingmoui.platform" VERSION 2.0 GENERATE_PLUGIN_SOURCE DEPENDENCIES QtQuick)

set_target_properties(LingmoUIPlatform PROPERTIES
    VERSION     ${PROJECT_VERSION}
    SOVERSION   6
    EXPORT_NAME "LingmoUIPlatform"
)

target_sources(LingmoUIPlatform PRIVATE
    platformtheme.cpp
    platformtheme.h
    basictheme.cpp
    basictheme_p.h
    inputmethod.cpp
    inputmethod.h
    platformpluginfactory.cpp
    platformpluginfactory.h
    tabletmodewatcher.cpp
    tabletmodewatcher.h
    settings.cpp
    settings.h
    smoothscrollwatcher.cpp
    smoothscrollwatcher.h
    styleselector.cpp
    styleselector.h
    units.cpp
    units.h
    virtualkeyboardwatcher.cpp
    virtualkeyboardwatcher.h
    colorutils.cpp
    colorutils.h
)

set(liblingmoui_extra_sources "")

if (WITH_DBUS)
    set_source_files_properties(org.freedesktop.portal.Settings.xml PROPERTIES INCLUDE dbustypes.h)
    qt_add_dbus_interface(liblingmoui_extra_sources org.freedesktop.portal.Settings.xml settings_interface)
    set(LIBKIRIGAMKI_EXTRA_LIBS Qt6::DBus)
endif()

target_sources(LingmoUIPlatform PRIVATE ${liblingmoui_extra_sources})

ecm_qt_declare_logging_category(LingmoUIPlatform
    HEADER lingmouiplatform_logging.h
    IDENTIFIER LingmoUIPlatform
    CATEGORY_NAME kf.lingmoui.platform
    DESCRIPTION "LingmoUI Platform"
    DEFAULT_SEVERITY Warning
    EXPORT LINGMOUI
)

ecm_setup_version(PROJECT
    VARIABLE_PREFIX LINGMOUIPLATFORM
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/lingmouiplatform_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6LingmoUIPlatformConfigVersion.cmake"
    SOVERSION 6
)

ecm_generate_export_header(LingmoUIPlatform
    VERSION ${PROJECT_VERSION}
    BASE_NAME LingmoUIPlatform
    USE_VERSION_HEADER
    DEPRECATION_VERSIONS
)

target_include_directories(LingmoUIPlatform
    PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
    "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}>"
    "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/LingmoUI/Platform>"
)

include_static_dependencies(LingmoUIPlatform EXPORT KF6LingmoUIPlatformTargets)

target_link_libraries(LingmoUIPlatform
    PUBLIC
    Qt6::Core
    Qt6::Qml
    Qt6::Quick
    PRIVATE
    Qt6::GuiPrivate
    Qt6::QuickControls2
    ${LIBKIRIGAMKI_EXTRA_LIBS}
)

ecm_generate_headers(LingmoUIPlatform_CamelCase_HEADERS
    HEADER_NAMES
    PlatformTheme
    PlatformPluginFactory
    StyleSelector
    TabletModeWatcher
    Units
    VirtualKeyboardWatcher
    REQUIRED_HEADERS LingmoUIPlatform_HEADERS
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KF6LingmoUIPlatformConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KF6LingmoUIPlatformConfig.cmake"
    INSTALL_DESTINATION ${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6LingmoUIPlatform
    PATH_VARS CMAKE_INSTALL_PREFIX
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/KF6LingmoUIPlatformConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/KF6LingmoUIPlatformConfigVersion.cmake"
    DESTINATION ${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6LingmoUIPlatform
    COMPONENT Devel
)

if (NOT BUILD_SHARED_LIBS)
    install(TARGETS LingmoUIPlatformplugin EXPORT KF6LingmoUIPlatformTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
endif()

install(TARGETS LingmoUIPlatform EXPORT KF6LingmoUIPlatformTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(EXPORT KF6LingmoUIPlatformTargets
    DESTINATION ${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6LingmoUIPlatform
    FILE KF6LingmoUIPlatformTargets.cmake
    NAMESPACE KF6::
)

install(FILES
    ${LingmoUIPlatform_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/lingmouiplatform_export.h
    ${CMAKE_CURRENT_BINARY_DIR}/lingmouiplatform_version.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/LingmoUI/Platform # prefix matching C++ namespace
    COMPONENT Devel
)
install(FILES
    ${LingmoUIPlatform_CamelCase_HEADERS}
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/LingmoUI/Platform # prefix matching C++ namespace
    COMPONENT Devel
)

ecm_qt_install_logging_categories(
    EXPORT LINGMOUI
    FILE lingmoui.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

ecm_finalize_qml_module(LingmoUIPlatform)

if(BUILD_QCH)
    ecm_add_qch(
        KF6LingmoUI_QCH
        NAME LingmoUIPlatform
        BASE_NAME KF6LingmoUIPlatform
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            platformpluginfactory.h
            platformtheme.h
            tabletmodewatcher.h
            units.h
            virtualkeyboardwatcher.h
        MD_MAINPAGE "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt6Core_QCH
        BLANK_MACROS
            LINGMOUIPLATFORM_EXPORT
            LINGMOUIPLATFORM_DEPRECATED
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

