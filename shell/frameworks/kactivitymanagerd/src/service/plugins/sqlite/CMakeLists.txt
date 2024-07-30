# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:
set (
   sqliteplugin_SRCS
   Database.cpp
   StatsPlugin.cpp
   ResourceScoreCache.cpp
   ResourceScoreMaintainer.cpp
   ResourceLinking.cpp

   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/database/Database.cpp
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/database/schema/ResourcesDatabaseSchema.cpp

   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/utils/qsqlquery_iterator.cpp
   )

qt_add_dbus_adaptor (
   sqliteplugin_SRCS
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.ResourcesLinking.xml
   ResourceLinking.h ResourceLinking
   resourcelinkingadaptor
   )

qt_add_dbus_adaptor (
   sqliteplugin_SRCS
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.ResourcesScoring.xml
   StatsPlugin.h StatsPlugin
   resourcescoringadaptor
   )

kamd_add_plugin(org.kde.ActivityManager.ResourceScoring SOURCES ${sqliteplugin_SRCS})

target_link_libraries(org.kde.ActivityManager.ResourceScoring 
   Qt::Sql
   KF6::KIOCore
   KF6::DBusAddons
   KF6::Crash
   debug_STATIC
)
