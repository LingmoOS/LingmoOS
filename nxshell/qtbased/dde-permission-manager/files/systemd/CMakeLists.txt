set(SERVICES
    dde-permission-manager.service
)

install(FILES ${SERVICES}
        DESTINATION lib/systemd/user/)
