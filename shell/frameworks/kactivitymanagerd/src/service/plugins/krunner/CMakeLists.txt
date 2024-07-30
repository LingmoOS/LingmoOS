# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:
qt_add_dbus_adaptor(activityrunner_DBUS_SRCS "org.kde.krunner1.xml" ActivityRunner.h ActivityRunner)

kamd_add_plugin(org.kde.ActivityManager.ActivityRunner SOURCES ActivityRunner.cpp ${activityrunner_DBUS_SRCS})

target_link_libraries(org.kde.ActivityManager.ActivityRunner
   KF6::I18n
   debug_STATIC
)

install(
   FILES plasma-runnners-activities.desktop
   DESTINATION "${KDE_INSTALL_DATAROOTDIR}/krunner/dbusplugins"
)


