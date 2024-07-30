# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

kamd_add_plugin(org.kde.ActivityManager.VirtualDesktopSwitch SOURCES VirtualDesktopSwitchPlugin.cpp)

target_link_libraries(org.kde.ActivityManager.VirtualDesktopSwitch
   KF6::WindowSystem
)

