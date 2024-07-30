add_definitions(-DTRANSLATION_DOMAIN=\"lingmo_engine_time\")

set(time_engine_SRCS
    timeengine.cpp
    timesource.cpp
    solarsystem.cpp
    timeengine.h
    timesource.h
    solarsystem.h
)

ecm_qt_declare_logging_category(time_engine_SRCS HEADER debug.h
                                               IDENTIFIER DATAENGINE_TIME
                                               CATEGORY_NAME kde.dataengine.time
                                               DEFAULT_SEVERITY Info
                                               DESCRIPTION "DataEngine time"
                                               EXPORT LINGMOWORKSPACE
                                           )

kcoreaddons_add_plugin(lingmo_engine_time SOURCES ${time_engine_SRCS} INSTALL_NAMESPACE lingmo5support/dataengine)

target_link_libraries(lingmo_engine_time
    Qt::DBus
    KF6::Solid
    Lingmo::Lingmo
    Lingmo::Lingmo5Support
    KF6::I18n
    KF6::Service
)
