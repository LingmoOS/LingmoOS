remove_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x50f02)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050e00) # needed for QNetworkConfigurationManager

set(plasma_geolocation_interface_SRCS geolocationprovider.cpp)
add_library(lingmo-geolocation-interface SHARED ${plasma_geolocation_interface_SRCS})
target_link_libraries(lingmo-geolocation-interface
    PUBLIC
        Qt::Core
        Qt::Network
        KF5::Plasma
    PRIVATE
        KF5::KIOCore
)
set_target_properties(lingmo-geolocation-interface PROPERTIES
   VERSION ${PROJECT_VERSION}
   SOVERSION ${PROJECT_VERSION_MAJOR}
)
install(TARGETS lingmo-geolocation-interface ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES geolocationprovider.h ${CMAKE_CURRENT_BINARY_DIR}/geolocation_export.h
        DESTINATION ${KDE_INSTALL_INCLUDEDIR}/plasma/geolocation
        COMPONENT Devel)

kcoreaddons_add_plugin(plasma_engine_geolocation SOURCES geolocation.cpp INSTALL_NAMESPACE plasma/dataengine)
target_compile_definitions(plasma_engine_geolocation PRIVATE -DQT_NO_KEYWORDS)
generate_export_header(plasma_engine_geolocation EXPORT_FILE_NAME "geolocation_export.h" BASE_NAME "GEOLOCATION")
target_link_libraries(plasma_engine_geolocation
    lingmo-geolocation-interface
    KF5::Plasma
    KF5::CoreAddons
    KF5::KIOCore
    KF5::NetworkManagerQt
    KF5::Service
    KF5::Solid)

kcoreaddons_add_plugin(lingmo-geolocation-ip SOURCES location_ip.cpp INSTALL_NAMESPACE plasma/geolocationprovider)
ecm_qt_declare_logging_category(lingmo-geolocation-ip HEADER geolocdebug.h IDENTIFIER DATAENGINE_GEOLOCATION CATEGORY_NAME org.kde.plasma.dataengine.geolocation)
target_compile_definitions(lingmo-geolocation-ip PRIVATE -DQT_NO_KEYWORDS)
target_link_libraries(lingmo-geolocation-ip lingmo-geolocation-interface KF5::NetworkManagerQt)

pkg_check_modules(LIBGPS libgps IMPORTED_TARGET)

if(TARGET PkgConfig::LIBGPS)
    kcoreaddons_add_plugin(lingmo-geolocation-gps SOURCES location_gps.cpp INSTALL_NAMESPACE plasma/geolocationprovider)
    ecm_qt_declare_logging_category(lingmo-geolocation-gps HEADER geolocdebug.h IDENTIFIER DATAENGINE_GEOLOCATION CATEGORY_NAME org.kde.plasma.dataengine.geolocation)
    target_link_libraries(lingmo-geolocation-gps lingmo-geolocation-interface PkgConfig::LIBGPS)
endif()
