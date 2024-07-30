# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:
kamd_add_plugin(org.kde.ActivityManager.LibreOfficeEventSpy SOURCES LibreOfficeEventSpy.cpp)

ecm_qt_declare_logging_category(org.kde.ActivityManager.LibreOfficeEventSpy
   HEADER DebugLibreOfficeEventSpy.h
   IDENTIFIER KAMD_LOG_PLUGIN_LIBREOFFICE_EVENTSPY
   CATEGORY_NAME org.kde.kactivities.plugin.libreoffice-eventspy
   DEFAULT_SEVERITY Warning
   DESCRIPTION "KActivities LibreOffice Event Spy Resources" EXPORT KACTIVITYMANAGERD)

target_link_libraries(org.kde.ActivityManager.LibreOfficeEventSpy
   Qt::Xml
   KF6::Service
)

