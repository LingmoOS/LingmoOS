set(FCITX5_SOURCES
  Fcitx5Proxy.h
  Fcitx5Proxy.cpp
  DBusProvider.h
  DBusProvider.cpp
)

add_library(fcitx5 MODULE
  ${FCITX5_SOURCES}
)

target_link_libraries(fcitx5 PRIVATE
  dimcore
  PkgConfig::wlroots
  Qt6::Core
  Qt6::DBus
  Qt6::GuiPrivate
  Fcitx5Qt6::DBusAddons
  $<$<BOOL:${Dtk6Core_FOUND}>:Dtk6::Core>
  wlc
  wl_addons_base
)

install(TARGETS fcitx5
  LIBRARY DESTINATION ${DIM_ADDON_DIR}
)

install(FILES fcitx5.conf
  DESTINATION ${DIM_ADDON_INFO_DIR}
)

