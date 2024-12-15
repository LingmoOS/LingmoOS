configure_file(
  com.lingmo.wm.service.in
  com.lingmo.wm.service
  ONLY
)
configure_file(
  org.lingmo.ocean.Appearance1.service.in
  org.lingmo.ocean.Appearance1.service
  ONLY
)

set(DBUS_SERVICE_FILES
  ${CMAKE_CURRENT_BINARY_DIR}/com.lingmo.wm.service
  ${CMAKE_CURRENT_BINARY_DIR}/org.lingmo.ocean.Appearance1.service
)

install(FILES ${DBUS_SERVICE_FILES} DESTINATION share/dbus-1/services/)
