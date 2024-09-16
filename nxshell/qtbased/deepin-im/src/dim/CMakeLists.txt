set(DIM_SOURCES
  main.cpp
  ./dbus/DimDBusType.cpp
  ./dbus/DimDBusAdaptor.cpp
)

set(DIM_HEADERS
  ./dbus/DimDBusType.h
  ./dbus/DimDBusAdaptor.h
)

qt_add_dbus_adaptor(DIM_SOURCES ${CMAKE_SOURCE_DIR}/misc/adaptor/org.deepin.dde.Dim.xml dbus/DimDBusAdaptor.h DimDBusAdaptor)

add_executable(dim
  ${DIM_SOURCES}
  ${DIM_HEADERS}
)

target_link_libraries(dim PRIVATE
  dimcore
  Qt6::Gui
  Qt6::DBus
  $<$<BOOL:${Dtk6Core_FOUND}>:Dtk6::Core>
)

install(TARGETS dim
  BUNDLE DESTINATION .
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
