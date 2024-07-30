# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

kamd_add_plugin(org.kde.ActivityManager.RunApplication SOURCES RunApplicationPlugin.cpp)

target_link_libraries(org.kde.ActivityManager.RunApplication
   Qt::Gui
   KF6::Service
   debug_STATIC
)

