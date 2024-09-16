set(SERVICES
    org.deepin.dde.Permission1.service
)

install(FILES ${SERVICES}
        DESTINATION share/dbus-1/services/)
