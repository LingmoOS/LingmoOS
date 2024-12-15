set(WLFRONTEND_SOURCES
  WLFrontend.h
  WLFrontend.cpp
  VirtualInputContextManager.h
  VirtualInputContextManager.cpp
  VirtualInputContext.h
  VirtualInputContext.cpp
  VirtualInputContextGlue.h
  VirtualInputContextGlue.cpp
  WaylandInputContext.h
  WaylandInputContext.cpp
  InputMethodV2.h
  InputMethodV2.cpp
  InputMethodKeyboardGrabV2.h
  InputMethodKeyboardGrabV2.cpp
  InputPopupSurfaceV2.h
  InputPopupSurfaceV2.cpp
  AppMonitor.h
  AppMonitor.cpp
  WlrAppMonitor.h
  WlrAppMonitor.cpp
  ForeignToplevelManagementV1.h
  ForeignToplevelManagementV1.cpp
  Xcb.h
  Xcb.cpp
  X11AppMonitor.h
  X11AppMonitor.cpp
  WLFrontend_public.h
  WLFrontend_public.cpp
  TreelandForeignToplevelManagementV1.h
  TreelandForeignToplevelManagementV1.cpp
)

add_library(wlfrontend SHARED
  ${WLFRONTEND_SOURCES}
)

target_link_libraries(wlfrontend PRIVATE
  dimcore
  Qt6::Core
  Qt6::DBus
  Qt6::GuiPrivate
  PkgConfig::wayland-client
  PkgConfig::xcb
  PkgConfig::xkbcommon
  $<$<BOOL:${Dtk6Core_FOUND}>:Dtk6::Core>
  wlc
)

install(TARGETS wlfrontend
  LIBRARY DESTINATION ${DIM_ADDON_DIR}
)

install(FILES wlfrontend.conf
  DESTINATION ${DIM_ADDON_INFO_DIR}
)
