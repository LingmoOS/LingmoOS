set(SYSTEMD_FILES
    ocean-display.service
)

install(FILES ${SYSTEMD_FILES} DESTINATION lib/systemd/user/)
