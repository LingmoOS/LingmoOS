set(KSYSGUARD_SENSORS_SOVERSION 2)

add_definitions(-DTRANSLATION_DOMAIN=\"ksysguard_sensors\")

set(sensors_LIB_SRCS
    Sensor.cpp
    SensorDataModel.cpp
    SensorTreeModel.cpp
    SensorGroup.cpp
    SensorQuery.cpp
    SensorDaemonInterface.cpp
    SensorUnitModel.cpp
)

set(sensors_LIB_HEADERS
    Sensor.h
    SensorDataModel.h
    SensorTreeModel.h
    SensorQuery.h
    SensorUnitModel.h
    SensorInfo_p.h
)

ecm_qt_declare_logging_category(sensors_LIB_SRCS
    HEADER sensors_logging.h
    IDENTIFIER LIBKSYSGUARD_SENSORS
    CATEGORY_NAME org.kde.libksysguard.sensors
    DESCRIPTION "libksysguard (sensors)" EXPORT LIBKSYSGUARD
)

set_source_files_properties(org.kde.KSysGuardDaemon.xml PROPERTIES INCLUDE ../systemstats/SensorInfo.h)
qt_add_dbus_interface(sensors_LIB_SRCS org.kde.KSysGuardDaemon.xml ksysguarddaemon)

add_library(Sensors ${sensors_LIB_SRCS})
add_library(KSysGuard::Sensors ALIAS Sensors)

ecm_add_qml_module(Sensors URI "org.kde.ksysguard.sensors" VERSION 1.0 GENERATE_PLUGIN_SOURCE DEPENDENCIES QtCore)
ecm_finalize_qml_module(Sensors)

target_include_directories(Sensors
    PUBLIC
    "$<BUILD_INTERFACE:${libksysguard_SOURCE_DIR}>"
    "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/ksysguard/>"
)

generate_export_header(Sensors)

target_link_libraries(Sensors
    PUBLIC
        Qt::Qml
        KSysGuard::Formatter
    PRIVATE
        Qt::Core
        Qt::DBus
        KF6::I18n
        KSysGuard::SystemStats
)

set_target_properties(Sensors PROPERTIES
    LIBRARY_OUTPUT_NAME KSysGuardSensors
    VERSION ${KSYSGUARD_VERSION}
    SOVERSION ${KSYSGUARD_SENSORS_SOVERSION}
)

install(TARGETS Sensors EXPORT libksysguardLibraryTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
    ${sensors_LIB_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/sensors_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/ksysguard/sensors
    COMPONENT Devel
)
