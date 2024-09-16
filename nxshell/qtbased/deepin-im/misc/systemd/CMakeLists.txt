if (NOT DEFINED SYSTEMD_USER_UNIT_DIR)
    pkg_get_variable(SYSTEMD_USER_UNIT_DIR systemd systemduserunitdir)
endif()

set(SERVICES
    org.deepin.dde.dim.target
    org.deepin.dde.dimwl@x11.service
    org.deepin.dde.dim@wayland.service
    org.deepin.dde.dim@x11.service
)

install(FILES ${SERVICES} DESTINATION ${SYSTEMD_USER_UNIT_DIR})
