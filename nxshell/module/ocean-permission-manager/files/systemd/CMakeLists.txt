set(SERVICES
    ocean-permission-manager.service
)

install(FILES ${SERVICES}
        DESTINATION lib/systemd/user/)
