# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:
kamd_add_plugin(org.kde.ActivityManager.GlobalShortcuts SOURCES GlobalShortcutsPlugin.cpp)

target_link_libraries (org.kde.ActivityManager.GlobalShortcuts
   Qt::Widgets
   KF6::GlobalAccel
   KF6::I18n
   KF6::XmlGui
)

