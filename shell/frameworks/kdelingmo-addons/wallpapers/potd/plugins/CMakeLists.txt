add_library(lingmo_wallpaper_potdplugin_debug STATIC)
set_property(TARGET lingmo_wallpaper_potdplugin_debug PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(lingmo_wallpaper_potdplugin_debug
    Qt::Core
)
ecm_qt_declare_logging_category(lingmo_wallpaper_potdplugin_debug
    HEADER debug.h
    IDENTIFIER WALLPAPERPOTD
    CATEGORY_NAME kde.wallpapers.potd
    DEFAULT_SEVERITY Info
    DESCRIPTION "Wallpaper plugins"
    EXPORT KDELINGMOADDONS
)

ecm_add_qml_module(lingmo_wallpaper_potdplugin URI org.kde.lingmo.wallpapers.potd)
target_sources(lingmo_wallpaper_potdplugin PRIVATE
    cachedprovider.cpp
    potdbackend.cpp
    potdengine.cpp
    potdprovidermodel.cpp
    potdplugin.cpp
)
target_link_libraries(lingmo_wallpaper_potdplugin PRIVATE lingmopotdprovidercore
    KF6::I18n
    KF6::KIOCore
    Qt::DBus
    Qt::Network
    Qt::Qml
    Qt::Widgets # QFileDialog
    lingmo_wallpaper_potdplugin_debug
)
ecm_finalize_qml_module(lingmo_wallpaper_potdplugin)

########### plugin core library ############
set(POTDPROVIDER_VERSION 2.0.0)
set(POTDPROVIDER_VERSION_MAJOR 2)

add_library(lingmopotdprovidercore SHARED potdprovider.cpp)
add_library(Lingmo::PotdProvider ALIAS lingmopotdprovidercore)
set_target_properties(lingmopotdprovidercore PROPERTIES
    VERSION ${POTDPROVIDER_VERSION}
    SOVERSION ${POTDPROVIDER_VERSION_MAJOR}
    EXPORT_NAME PotdProvider
)
target_link_libraries( lingmopotdprovidercore Qt::Gui KF6::CoreAddons KF6::ConfigCore KF6::KIOCore)
target_include_directories(lingmopotdprovidercore
    PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}>"
)
ecm_generate_export_header(lingmopotdprovidercore
    BASE_NAME LINGMO_POTD
    EXPORT_FILE_NAME lingmo_potd_export.h
    VERSION ${POTDPROVIDER_VERSION}
    DEPRECATED_BASE_VERSION 0
)

install(TARGETS lingmopotdprovidercore EXPORT lingmopotdproviderTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
        potdprovider.h
        ${CMAKE_CURRENT_BINARY_DIR}/lingmo_potd_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/lingmo/potdprovider
    COMPONENT Devel
)

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/LingmoPotdProviderConfigVersion.cmake
    VERSION "${POTDPROVIDER_VERSION}"
    COMPATIBILITY SameMajorVersion
)

set(CMAKECONFIG_INSTALL_DIR ${KDE_INSTALL_CMAKEPACKAGEDIR}/LingmoPotdProvider)
configure_package_config_file(LingmoPotdProvider.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/LingmoPotdProviderConfig.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/LingmoPotdProviderConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/LingmoPotdProviderConfigVersion.cmake
    DESTINATION ${CMAKECONFIG_INSTALL_DIR}
    COMPONENT Devel
)

install(EXPORT lingmopotdproviderTargets
    NAMESPACE Lingmo::
    DESTINATION ${CMAKECONFIG_INSTALL_DIR}
    FILE LingmoPotdProviderTargets.cmake
    COMPONENT Devel
)

add_subdirectory(providers)
