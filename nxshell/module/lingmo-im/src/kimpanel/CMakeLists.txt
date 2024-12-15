set(KIMPANEL_SOURCES
  main.cpp
  Panel.h
  Panel.cpp
  qml.qrc
)

qt_add_dbus_adaptor(KIMPANEL_SOURCES org.kde.impanel.xml Panel.h Panel ImpanelAdaptor)
qt_add_dbus_adaptor(KIMPANEL_SOURCES org.kde.impanel2.xml Panel.h Panel Impanel2Adaptor)
qt_add_dbus_interface(KIMPANEL_SOURCES org.kde.impanel.inputmethod.xml ImpanelInputmethod)

add_executable(kimpanel
  ${KIMPANEL_SOURCES}
)

target_link_libraries(kimpanel PRIVATE
  Qt6::Core
  Qt6::Quick
  Qt6::DBus
)

install(TARGETS kimpanel
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
