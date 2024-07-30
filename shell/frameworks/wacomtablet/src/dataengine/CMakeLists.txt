add_definitions(-DTRANSLATION_DOMAIN=\"wacomtablet\")

set(dataengine_SRCS
    wacomtabletengine.cpp
    wacomtabletservice.cpp
    multidbuspendingcallwatcher.cpp

    wacomtabletengine.h
    wacomtabletservice.h
    multidbuspendingcallwatcher.h
)

add_library(lingmo_engine_wacomtablet MODULE ${dataengine_SRCS})
target_link_libraries(lingmo_engine_wacomtablet
                      Lingmo::Lingmo
                      Qt::DBus
                      wacom_common
)

target_link_libraries(lingmo_engine_wacomtablet Lingmo::Lingmo5Support)

install(TARGETS lingmo_engine_wacomtablet
        DESTINATION ${KDE_INSTALL_PLUGINDIR}/lingmo5support/dataengine
)
install(FILES wacomtablet.operations
        DESTINATION ${KDE_INSTALL_DATADIR}/lingmo5support/services
)
