set(SERVICES
    org.lingmo.ocean.Permission1.service
)

install(FILES ${SERVICES}
        DESTINATION share/dbus-1/services/)
