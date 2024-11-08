set(TRAY_SRCS
    statusnotifieritemjob.cpp
    statusnotifieritemsource.cpp
    systemtraytypes.cpp
    systemtraytypedefs.h
)

set(statusnotifierwatcher_xml org.kde.StatusNotifierWatcher.xml)
qt5_add_dbus_interface(SRCS ${statusnotifierwatcher_xml} statusnotifierwatcher_interface)
qt5_add_dbus_interface(SRCS org.freedesktop.DBus.Properties.xml dbusproperties)

set(statusnotifieritem_xml org.kde.StatusNotifierItem.xml)
set_source_files_properties(${statusnotifieritem_xml} PROPERTIES
   NO_NAMESPACE false
   INCLUDE "systemtraytypes.h"
   CLASSNAME OrgKdeStatusNotifierItem
)
qt5_add_dbus_interface(SRCS ${statusnotifieritem_xml} statusnotifieritem_interface)