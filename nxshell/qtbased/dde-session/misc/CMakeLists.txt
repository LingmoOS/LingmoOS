configure_file(xsessions/dde-x11.desktop.in xsessions/dde-x11.desktop)

set(XSESSION
    ${CMAKE_CURRENT_SOURCE_DIR}/Xsession.d/00deepin-dde-env
    ${CMAKE_CURRENT_SOURCE_DIR}/Xsession.d/01deepin-profile
    ${CMAKE_CURRENT_SOURCE_DIR}/Xsession.d/94qt_env
)

set(PROFILE
    ${CMAKE_CURRENT_SOURCE_DIR}/profile.d/deepin-xdg-dir.sh
)

install(
    DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR}/xsessions
    DESTINATION
        ${CMAKE_INSTALL_DATADIR}
    PATTERN
        "*.in" EXCLUDE
)

install(FILES ${XSESSION} DESTINATION /etc/X11/Xsession.d/)
install(FILES ${PROFILE} DESTINATION /etc/profile.d)
