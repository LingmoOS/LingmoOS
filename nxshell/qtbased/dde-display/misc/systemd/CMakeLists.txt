set(SYSTEMD_FILES
    dde-display.service
)

install(FILES ${SYSTEMD_FILES} DESTINATION lib/systemd/user/)
