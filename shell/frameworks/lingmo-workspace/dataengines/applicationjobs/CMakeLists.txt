add_definitions(-DTRANSLATION_DOMAIN=\"lingmo_engine_applicationjobs\")

set(kuiserver_engine_SRCS
    kuiserverengine.cpp
    jobcontrol.cpp
    jobaction.cpp
)

add_library(lingmo_engine_applicationjobs MODULE ${kuiserver_engine_SRCS})
target_link_libraries(lingmo_engine_applicationjobs
  Qt::DBus
  KF6::CoreAddons
  KF6::I18n
  KF6::KIOCore
  Lingmo::Lingmo
  Lingmo::Lingmo5Support
  KF6::Service
  PW::LibNotificationManager
)

install(TARGETS lingmo_engine_applicationjobs DESTINATION ${KDE_INSTALL_PLUGINDIR}/lingmo5support/dataengine)
install(FILES applicationjobs.operations DESTINATION ${LINGMO5SUPPORT_DATA_INSTALL_DIR}/services)
