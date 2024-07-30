add_definitions(-DTRANSLATION_DOMAIN=\"lingmo_engine_hotplug\")

set(hotplug_engine_SRCS
    deviceserviceaction.cpp
    hotplugengine.cpp
    hotplugservice.cpp
    hotplugjob.cpp
    deviceserviceaction.h
    hotplugengine.h
    hotplugservice.h
    hotplugjob.h
)

kcoreaddons_add_plugin(lingmo_engine_hotplug SOURCES ${hotplug_engine_SRCS} INSTALL_NAMESPACE lingmo5support/dataengine)
target_link_libraries(lingmo_engine_hotplug
    KF6::CoreAddons
    Lingmo::Lingmo
    Lingmo::Lingmo5Support
    KF6::Solid
    KF6::Service
    KF6::KIOCore
    KF6::KIOGui # KIO::CommandLauncherJob
    KF6::JobWidgets # KNotificationJobUiDelegate
    KF6::Notifications
    KF6::I18n
)

install(FILES hotplug.operations DESTINATION ${LINGMO5SUPPORT_DATA_INSTALL_DIR}/services)
