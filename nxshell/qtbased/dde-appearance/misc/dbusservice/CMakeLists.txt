configure_file(
  com.deepin.wm.service.in
  com.deepin.wm.service
  ONLY
)
configure_file(
  org.deepin.dde.Appearance1.service.in
  org.deepin.dde.Appearance1.service
  ONLY
)

set(DBUS_SERVICE_FILES
  ${CMAKE_CURRENT_BINARY_DIR}/com.deepin.wm.service
  ${CMAKE_CURRENT_BINARY_DIR}/org.deepin.dde.Appearance1.service
)

install(FILES ${DBUS_SERVICE_FILES} DESTINATION share/dbus-1/services/)
