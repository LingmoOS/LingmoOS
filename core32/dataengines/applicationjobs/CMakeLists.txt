add_definitions(-DTRANSLATION_DOMAIN=\"plasma_engine_applicationjobs\")

set(kuiserver_engine_SRCS
    kuiserverengine.cpp
    jobcontrol.cpp
    jobaction.cpp
)

add_library(plasma_engine_applicationjobs MODULE ${kuiserver_engine_SRCS})
target_link_libraries(plasma_engine_applicationjobs
  Qt::DBus
  KF5::CoreAddons
  KF5::I18n
  KF5::KIOCore
  KF5::Plasma
  KF5::Service
  PW::LibNotificationManager
)

install(TARGETS plasma_engine_applicationjobs DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/dataengine)
install(FILES applicationjobs.operations DESTINATION ${PLASMA_DATA_INSTALL_DIR}/services)
