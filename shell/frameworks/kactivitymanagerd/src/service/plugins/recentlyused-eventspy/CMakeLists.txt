# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:
kamd_add_plugin(org.kde.ActivityManager.RecentlyUsedEventSpy SOURCES RecentlyUsedEventSpy.cpp)

ecm_qt_declare_logging_category(org.kde.ActivityManager.RecentlyUsedEventSpy
   HEADER DebugPluginRecentlyUsedEventSpy.h
   IDENTIFIER KAMD_LOG_PLUGIN_RECENTLYUSED_EVENTSPY
   CATEGORY_NAME org.kde.kactivities.plugin.recentlyused-eventspy
   DEFAULT_SEVERITY Warning
   DESCRIPTION "KActivities RecentlyUsed Event Spy Resources" EXPORT KACTIVITYMANAGERD)

target_link_libraries(org.kde.ActivityManager.RecentlyUsedEventSpy
   Qt::Xml
   KF6::Service
)

