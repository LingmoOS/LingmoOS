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

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui DBus XkbCommonSupport)
find_package(Qt5Gui ${REQUIRED_QT5_VERSION} REQUIRED Private)

add_library(dimplatforminputcontextplugin MODULE
  ${QTIMMODULE_SOURCES}
)

target_link_libraries(dimplatforminputcontextplugin PRIVATE
  PkgConfig::wayland-client
  Qt5::DBus
  Qt5::GuiPrivate
  Qt5::XkbCommonSupportPrivate
  wlc
  shm_open_anon
)

include(ECMQueryQt)
ecm_query_qt(_QT5PLUGINDIR QT_INSTALL_PLUGINS)
set(CMAKE_INSTALL_QT5PLUGINDIR ${_QT5PLUGINDIR} CACHE PATH "Qt5 plugin dir")
install(TARGETS dimplatforminputcontextplugin
  DESTINATION ${CMAKE_INSTALL_QT5PLUGINDIR}/platforminputcontexts
)
