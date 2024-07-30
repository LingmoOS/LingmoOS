set(SystemStats_SOVERSION 2)

add_compile_definitions(TRANSLATION_DOMAIN="ksysguard_systemstats")

set(systemstats_SRCS
    AggregateSensor.cpp
    SensorObject.cpp
    SensorContainer.cpp
    SensorPlugin.cpp
    SensorProperty.cpp
    SysFsSensor.cpp
    DBusInterface.cpp
)

set(systemstats_HDRS
    AggregateSensor.h
    SensorObject.h
    SensorContainer.h
    SensorPlugin.h
    SensorProperty.h
    SensorsFeatureSensor.h
    SysFsSensor.h
    SysctlSensor.h
    SensorInfo.h
    DBusInterface.h
)

set_source_files_properties(org.kde.ksystemstats1.xml PROPERTIES INCLUDE SensorInfo.h)
qt_add_dbus_interface(dbusinterface_SRCS org.kde.ksystemstats1.xml org.kde.ksystemstats1)

# Symbol visibility trickery: We cannot generate the DBus interface class with
# the right export macro, so instead create it as a static library with the right
# visibility preset set, then link to that in the shared library.
add_library(dbusinterface STATIC ${dbusinterface_SRCS})
set_target_properties(dbusinterface PROPERTIES CXX_VISIBILITY_PRESET default)
target_link_libraries(dbusinterface Qt::Core Qt::DBus KSysGuard::Formatter)
set_property(TARGET dbusinterface PROPERTY POSITION_INDEPENDENT_CODE ON)

add_library(SystemStats ${systemstats_SRCS})
add_library(KSysGuard::SystemStats ALIAS SystemStats)

ecm_qt_declare_logging_category(SystemStats
    HEADER systemstats_logging.h
    IDENTIFIER SYSTEMSTATS
    CATEGORY_NAME org.kde.libksysguard.systemstats
    DESCRIPTION "libksysguard (system stats)"
    EXPORT LIBKSYSGUARD
)

target_include_directories(SystemStats
    PUBLIC
    "$<BUILD_INTERFACE:${libksysguard_SOURCE_DIR}>"
    "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/ksysguard/>"
)

target_link_libraries(SystemStats PUBLIC Qt::Core Qt::DBus KSysGuard::Formatter PRIVATE dbusinterface)

if (SENSORS_FOUND)
    target_sources(SystemStats PRIVATE SensorsFeatureSensor.cpp)
    target_link_libraries(SystemStats PUBLIC ${SENSORS_LIBRARIES} KF6::I18n)
    target_include_directories(SystemStats PUBLIC ${SENSORS_INCLUDE_DIR})
endif()

generate_export_header(SystemStats)

set_target_properties(SystemStats PROPERTIES
    LIBRARY_OUTPUT_NAME KSysGuardSystemStats
    VERSION ${KSYSGUARD_VERSION}
    SOVERSION ${SystemStats_SOVERSION}
)

install(TARGETS SystemStats EXPORT libksysguardLibraryTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${systemstats_HDRS}
    ${CMAKE_CURRENT_BINARY_DIR}/systemstats_export.h
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.ksystemstats1.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/ksysguard/systemstats
)

install(FILES org.kde.ksystemstats1.xml DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR})
