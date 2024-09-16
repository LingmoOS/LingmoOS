set(WL_ADDONS_BASE_SOURCES
  Global.h
  Listener.h
  Server.h
  Server.cpp
  Output.h
  Output.cpp
  View.h
  View.cpp
  Keyboard.h
  Keyboard.cpp
  InputMethodV2.h
  InputMethodV2.cpp
  InputPopupSurfaceV2.cpp
  InputPopupSurfaceV2.h
  InputMethodContextV1.h
  InputMethodContextV1.cpp
  InputMethodV1.h
  InputMethodV1.cpp

  inputmethodv1/Type.h
  inputmethodv1/Resource.h
  inputmethodv1/Resource.cpp
  inputmethodv1/ZwpInputMethodContextV1.cpp
  inputmethodv1/ZwpInputMethodContextV1.h
  inputmethodv1/ZwpInputMethodV1.cpp
  inputmethodv1/ZwpInputMethodV1.h
  inputmethodv1/ZwpInputPanelSurfaceV1.cpp
  inputmethodv1/ZwpInputPanelSurfaceV1.h
  inputmethodv1/ZwpInputPanelV1.cpp
  inputmethodv1/ZwpInputPanelV1.h
  inputmethodv1/InputMethodGrabV1.cpp
  inputmethodv1/InputMethodGrabV1.h
)

include(${PROJECT_SOURCE_DIR}/cmake/WaylandScannerHelpers.cmake)

ws_generate(
    server
    wayland-protocols
    stable/xdg-shell/xdg-shell.xml
    xdg-shell-protocol
)

add_library(wl_addons_base STATIC)

target_sources(wl_addons_base
  PRIVATE ${WL_ADDONS_BASE_SOURCES}
)

target_compile_definitions(wl_addons_base PRIVATE -DQT_NO_KEYWORDS)

target_link_libraries(wl_addons_base PRIVATE
  dimcore
  PkgConfig::wlroots
  PkgConfig::wayland-server
  wlc
  Qt6::Core
  shm_open_anon
)
