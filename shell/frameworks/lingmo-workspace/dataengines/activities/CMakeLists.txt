set(activity_engine_SRCS
        ActivityData.cpp
        activityengine.cpp
        activityservice.cpp
        activityjob.cpp)

set_source_files_properties(org.kde.ActivityManager.ActivityRanking.xml PROPERTIES INCLUDE "ActivityData.h")
qt_add_dbus_interface(
   activity_engine_SRCS org.kde.ActivityManager.ActivityRanking.xml
   ActivityRankingInterface
   )

add_library(lingmo_engine_activities MODULE ${activity_engine_SRCS})
target_link_libraries(lingmo_engine_activities
                      KF6::CoreAddons
                      Lingmo::Lingmo
                      Lingmo::Lingmo5Support
                      Lingmo::Activities
                      KF6::I18n
                      KF6::Service
                      Qt::DBus
                      Qt::Widgets
                      )

install(TARGETS lingmo_engine_activities
        DESTINATION ${KDE_INSTALL_PLUGINDIR}/lingmo5support/dataengine)

install(FILES activities.operations
        DESTINATION ${LINGMO5SUPPORT_DATA_INSTALL_DIR}/services)
