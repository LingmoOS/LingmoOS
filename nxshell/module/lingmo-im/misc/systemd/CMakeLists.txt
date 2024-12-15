if (NOT DEFINED SYSTEMD_USER_UNIT_DIR)
    pkg_get_variable(SYSTEMD_USER_UNIT_DIR systemd systemduserunitdir)
endif()

set(SERVICES
    org.lingmo.ocean.dim.target
    org.lingmo.ocean.dimwl@x11.service
    org.lingmo.ocean.dim@wayland.service
    org.lingmo.ocean.dim@x11.service
)

install(FILES ${SERVICES} DESTINATION ${SYSTEMD_USER_UNIT_DIR})
