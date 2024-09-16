set(dbus_SRCS)
set_source_files_properties(
  ${CMAKE_CURRENT_LIST_DIR}/org.desktopspec.ConfigManager.xml
  PROPERTIES
  NO_NAMESPACE ON
  CLASSNAME DSGConfig
)

if("${QT_VERSION_MAJOR}" STREQUAL "6")
  qt_add_dbus_interface(dbus_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/org.desktopspec.ConfigManager.xml
    configmanager_interface
  )
else()
  qt5_add_dbus_interface(dbus_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/org.desktopspec.ConfigManager.xml
    configmanager_interface
  )
endif()

set_source_files_properties(
  ${CMAKE_CURRENT_LIST_DIR}/org.desktopspec.ConfigManager.Manager.xml 
  PROPERTIES 
  NO_NAMESPACE ON 
  CLASSNAME DSGConfigManager
)

if("${QT_VERSION_MAJOR}" STREQUAL "6")
  qt_add_dbus_interface(dbus_SRCS
    ${CMAKE_CURRENT_LIST_DIR}/org.desktopspec.ConfigManager.Manager.xml
    manager_interface
  )
else()
  qt5_add_dbus_interface(dbus_SRCS
  ${CMAKE_CURRENT_LIST_DIR}/org.desktopspec.ConfigManager.Manager.xml
  manager_interface
  )
endif()
