set(DBUS_SERVICE_FILES
    org.deepin.dde.Display1.service
)

install(FILES ${DBUS_SERVICE_FILES} DESTINATION share/dbus-1/services/)
