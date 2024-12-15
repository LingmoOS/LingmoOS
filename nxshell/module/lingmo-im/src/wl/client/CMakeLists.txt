set(WLC_SOURCES
  ConnectionBase.h
  ConnectionBase.cpp
  ConnectionRaw.h
  ConnectionRaw.cpp
  Connection.h
  Connection.cpp
  Compositor.h Compositor.cpp
  Surface.h Surface.cpp
  Shm.h
  Shm.cpp
  Seat.h
  Seat.cpp
  Keyboard.h
  Keyboard.cpp
  XdgWmBase.h
  XdgWmBase.cpp
  XdgSurface.h
  XdgSurface.cpp
  XdgToplevel.h
  XdgToplevel.cpp
  ZwpInputMethodManagerV2.h
  ZwpInputMethodManagerV2.cpp
  ZwpInputMethodV2.h
  ZwpInputMethodV2.cpp
  ZwpInputMethodKeyboardGrabV2.h
  ZwpInputMethodKeyboardGrabV2.cpp
  ZwpInputPopupSurfaceV2.h
  ZwpInputPopupSurfaceV2.cpp
  ZwpVirtualKeyboardManagerV1.h
  ZwpVirtualKeyboardManagerV1.cpp
  ZwpVirtualKeyboardV1.h
  ZwpVirtualKeyboardV1.cpp
  ZwpTextInputManagerV3.h
  ZwpTextInputManagerV3.cpp
  ZwpTextInputV3.h
  ZwpTextInputV3.cpp
  ZwlrForeignToplevelManagementV1.h
  ZwlrForeignToplevelManagementV1.cpp
  ZtreelandForeignToplevelManagementV1.h
  ZtreelandForeignToplevelManagementV1.cpp
)

add_library(wlc STATIC)

target_sources(wlc
  PRIVATE ${WLC_SOURCES}
)

target_compile_definitions(wlc PRIVATE -DQT_NO_KEYWORDS)

target_link_libraries(wlc PRIVATE
  PkgConfig::wayland-client
  wayland_protocols
)
