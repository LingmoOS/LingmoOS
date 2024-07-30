add_library(plasma_wallpaper_potdplugin_debug STATIC)
set_property(TARGET plasma_wallpaper_potdplugin_debug PROPERTY POSITION_INDEPENDENT_CODE ON)
target_link_libraries(plasma_wallpaper_potdplugin_debug
    Qt::Core
)
ecm_qt_declare_logging_category(plasma_wallpaper_potdplugin_debug
    HEADER debug.h
    IDENTIFIER WALLPAPERPOTD
    CATEGORY_NAME kde.wallpapers.potd
    DEFAULT_SEVERITY Info
    DESCRIPTION "Wallpaper plugins"
    EXPORT KDEPLASMAADDONS
)

ecm_add_qml_module(plasma_wallpaper_potdplugin URI org.kde.plasma.wallpapers.potd)
target_sources(plasma_wallpaper_potdplugin PRIVATE
    cachedprovider.cpp
    potdbackend.cpp
    potdengine.cpp
    potdprovidermodel.cpp
    potdplugin.cpp
)
target_link_libraries(plasma_wallpaper_potdplugin PRIVATE plasmapotdprovidercore
    KF6::I18n
    KF6::KIOCore
    Qt::DBus
    Qt::Network
    Qt::Qml
    Qt::Widgets # QFileDialog
    plasma_wallpaper_potdplugin_debug
)
ecm_finalize_qml_module(plasma_wallpaper_potdplugin)

########### plugin core library ############
set(POTDPROVIDER_VERSION 2.0.0)
set(POTDPROVIDER_VERSION_MAJOR 2)

add_library(plasmapotdprovidercore SHARED potdprovider.cpp)
add_library(Plasma::PotdProvider ALIAS plasmapotdprovidercore)
set_target_properties(plasmapotdprovidercore PROPERTIES
    VERSION ${POTDPROVIDER_VERSION}
    SOVERSION ${POTDPROVIDER_VERSION_MAJOR}
    EXPORT_NAME PotdProvider
)
target_link_libraries( plasmapotdprovidercore Qt::Gui KF6::CoreAddons KF6::ConfigCore KF6::KIOCore)
target_include_directories(plasmapotdprovidercore
    PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}>"
)
ecm_generate_export_header(plasmapotdprovidercore
    BASE_NAME PLASMA_POTD
    EXPORT_FILE_NAME plasma_potd_export.h
    VERSION ${POTDPROVIDER_VERSION}
    DEPRECATED_BASE_VERSION 0
)

install(TARGETS plasmapotdprovidercore EXPORT plasmapotdproviderTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
        potdprovider.h
        ${CMAKE_CURRENT_BINARY_DIR}/plasma_potd_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/plasma/potdprovider
    COMPONENT Devel
)

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/PlasmaPotdProviderConfigVersion.cmake
    VERSION "${POTDPROVIDER_VERSION}"
    COMPATIBILITY SameMajorVersion
)

set(CMAKECONFIG_INSTALL_DIR ${KDE_INSTALL_CMAKEPACKAGEDIR}/PlasmaPotdProvider)
configure_package_config_file(PlasmaPotdProvider.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/PlasmaPotdProviderConfig.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/PlasmaPotdProviderConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/PlasmaPotdProviderConfigVersion.cmake
    DESTINATION ${CMAKECONFIG_INSTALL_DIR}
    COMPONENT Devel
)

install(EXPORT plasmapotdproviderTargets
    NAMESPACE Plasma::
    DESTINATION ${CMAKECONFIG_INSTALL_DIR}
    FILE PlasmaPotdProviderTargets.cmake
    COMPONENT Devel
)

add_subdirectory(providers)
