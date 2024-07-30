set(apps_engine_SRCS
    appsengine.cpp
    appsource.cpp
    appservice.cpp
    appjob.cpp
    appsengine.h
    appsource.h
    appservice.h
    appjob.h
)

add_library(lingmo_engine_apps MODULE ${apps_engine_SRCS})

target_link_libraries(lingmo_engine_apps
    Lingmo::Lingmo
    Lingmo::Lingmo5Support
    KF6::Service
    KF6::KIOCore
    KF6::KIOWidgets
)

install(TARGETS lingmo_engine_apps DESTINATION ${KDE_INSTALL_PLUGINDIR}/lingmo5support/dataengine)
install(FILES apps.operations DESTINATION ${LINGMO5SUPPORT_DATA_INSTALL_DIR}/services)
