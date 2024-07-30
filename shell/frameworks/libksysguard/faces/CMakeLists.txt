set(KSYSGUARD_SENSORS_SOVERSION 2)

add_definitions(-DTRANSLATION_DOMAIN=\"KSysGuardSensorFaces\")

add_subdirectory(facepackages)
add_subdirectory(packagestructure)

set(faces_LIB_SRCS
    SensorFaceController.cpp
    SensorFace.cpp
    FaceLoader.cpp
    resources.qrc
)

set(faces_LIB_HEADERS
    SensorFace_p.h
    SensorFaceController.h
    FaceLoader.h
)

ecm_qt_declare_logging_category(faces_LIB_SRCS
    HEADER faces_logging.h
    IDENTIFIER LIBKSYSGUARD_SENSORS
    CATEGORY_NAME org.kde.libksysguard.faces
    DESCRIPTION "libksysguard (faces)" EXPORT LIBKSYSGUARD)

set_source_files_properties(org.kde.KSysGuardDaemon.xml PROPERTIES INCLUDE SensorInfo_p.h)

add_library(SensorFaces ${faces_LIB_SRCS} ${faces_LIB_HEADERS})
add_library(KSysGuard::SensorFaces ALIAS SensorFaces)

ecm_add_qml_module(SensorFaces URI "org.kde.ksysguard.faces"
    VERSION 1.0
    GENERATE_PLUGIN_SOURCE
    DEPENDENCIES
    "QtCore"
    "QtQuick"
    "org.kde.config"
)

ecm_target_qml_sources(SensorFaces SOURCES
    import/ExtendedLegend.qml
    import/SensorFace.qml
    import/SensorRangeSpinBox.qml
    import/Choices.qml
    import/CompactSensorFace.qml
)

ecm_finalize_qml_module(SensorFaces)

ecm_add_qml_module(SensorFacesPrivate URI "org.kde.ksysguard.faces.private"
    VERSION 1.0
    GENERATE_PLUGIN_SOURCE
)

target_sources(SensorFacesPrivate PRIVATE private_types.h)

ecm_finalize_qml_module(SensorFacesPrivate)

target_include_directories(SensorFaces
    PUBLIC
    "$<BUILD_INTERFACE:${libksysguard_SOURCE_DIR}>"
    "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/ksysguard/>"
)

generate_export_header(SensorFaces)

target_link_libraries(SensorFaces
    PUBLIC
        Qt::Qml
        Qt::Quick
        KF6::ConfigCore
        KF6::ConfigGui
        KSysGuard::Formatter
        KSysGuard::Sensors
    PRIVATE
        Qt::Core
        Qt::DBus
        KF6::I18n
        KF6::Package
        KF6::ConfigQml
        KF6::Solid
)

set_target_properties(SensorFaces PROPERTIES
    LIBRARY_OUTPUT_NAME KSysGuardSensorFaces
    VERSION ${KSYSGUARD_VERSION}
    SOVERSION ${KSYSGUARD_SENSORS_SOVERSION}
)

install(TARGETS SensorFaces EXPORT libksysguardLibraryTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES
    ${faces_LIB_HEADERS}
    ${CMAKE_CURRENT_BINARY_DIR}/sensorfaces_export.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/ksysguard/faces
    COMPONENT Devel
)

install(FILES systemmonitor-faces.knsrc systemmonitor-presets.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})
