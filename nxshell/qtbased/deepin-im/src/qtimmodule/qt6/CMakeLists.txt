set(QTIMMODULE_SOURCES
  DIMPlatformInputContextPlugin.h
  DIMPlatformInputContextPlugin.cpp
  DIMPlatformInputContext.h
  DIMPlatformInputContext.cpp
  Keyboard.h
  Keyboard.cpp
  TextInputV3.h
  TextInputV3.cpp
  XdgSurface.h
  XdgSurface.cpp
  XdgToplevel.h
  XdgToplevel.cpp

  QWaylandTextInputInterface.h
  QWaylandInputMethodEventBuilder.h
  QWaylandInputMethodEventBuilder.cpp
)

add_library(dimplatforminputcontextplugin-qt6 MODULE
  ${QTIMMODULE_SOURCES}
)

set_target_properties(dimplatforminputcontextplugin-qt6 PROPERTIES
  OUTPUT_NAME dimplatforminputcontextplugin
  AUTOMOC TRUE
)

target_include_directories(dimplatforminputcontextplugin-qt6  PRIVATE
)

target_link_libraries(dimplatforminputcontextplugin-qt6 PRIVATE
  PkgConfig::wayland-client
  Qt6::GuiPrivate
  Qt6::DBus
  wlc
  shm_open_anon
)

get_target_property(_QT6_QMAKE_EXECUTABLE Qt6::qmake LOCATION)
execute_process(
  COMMAND ${_QT6_QMAKE_EXECUTABLE} -query "QT_INSTALL_PLUGINS"
  RESULT_VARIABLE return_code
  OUTPUT_VARIABLE _QT6PLUGINDIR
)

if(return_code EQUAL 0)
  string(STRIP "${_QT6PLUGINDIR}" _QT6PLUGINDIR)
else()
  message(FATAL_ERROR "QMake Qt6 call failed: ${return_code}")
endif()

set(CMAKE_INSTALL_QT6PLUGINDIR ${_QT6PLUGINDIR} CACHE PATH "Qt6 plugin dir")
install(TARGETS dimplatforminputcontextplugin-qt6
  DESTINATION ${CMAKE_INSTALL_QT6PLUGINDIR}/platforminputcontexts
)
