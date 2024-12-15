include(GNUInstallDirs)

file(GLOB DBusAPI ${CMAKE_CURRENT_LIST_DIR}/dbus/*.xml)

configure_file(
    ${CMAKE_SOURCE_DIR}/api/OCEANApplicationManagerConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/OCEANApplicationManagerConfig.cmake
    @ONLY)

install(FILES ${DBusAPI}
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/ocean-application-manager/)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/OCEANApplicationManagerConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/OCEANApplicationManager/)
