if (NOT DEFINED SYSTEMD_SYSTEM_UNIT_DIR)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(Systemd REQUIRED systemd)
    pkg_get_variable(SYSTEMD_SYSTEM_UNIT_DIR systemd systemduserunitdir)
endif()

install(FILES override.conf DESTINATION ${SYSTEMD_SYSTEM_UNIT_DIR}/seatd.service.d/)
