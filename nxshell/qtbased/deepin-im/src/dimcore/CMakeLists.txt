configure_file(
  config.h.in
  config.h
)

set(DIMCORE_SOURCES
  Dim.cpp
  Addon.cpp
  FrontendAddon.cpp
  InputContext.cpp
  InputMethodAddon.cpp
  InputMethodEntry.cpp
  InputState.cpp
  Events.cpp
  ProxyAddon.cpp
  Dconfig.cpp
)

set(DIMCORE_HEADERS
  Dim.h
  Addon.h
  FrontendAddon.h
  SurroundingText.h
  InputContext.h
  InputMethodAddon.h
  InputMethodEntry.h
  InputState.h
  Events.h
  ProxyAddon.h
  Dconfig.h
)

add_library(dimcore SHARED
  ${DIMCORE_SOURCES}
  ${DIMCORE_HEADERS}
)

target_link_libraries(dimcore PRIVATE
  Qt6::Core
  Qt6::Quick
  Qt6::DBus
  ${CMAKE_DL_LIBS}
  $<$<BOOL:${Dtk6Core_FOUND}>:Dtk6::Core>
)

install(TARGETS dimcore
  BUNDLE DESTINATION .
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
