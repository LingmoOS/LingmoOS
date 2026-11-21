project(lingmo-locales)
set(TARGET lingmo-locales)

set(SOURCES
    localesservice.cpp
    localesservice.h
    main.cpp
)

add_executable(${TARGET} ${SOURCES})
target_link_libraries(${TARGET}
    Lingmo::SDKBase
    lingmo-core-common
)

configure_file(org.lingmo.Locales.service.in org.lingmo.Locales.service @ONLY)
configure_file(lingmo-locales.service.in lingmo-locales.service @ONLY)

install(TARGETS ${TARGET} DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
install(FILES org.lingmo.Locales.conf DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/dbus-1/system.d)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.lingmo.Locales.service
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/dbus-1/system-services)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lingmo-locales.service DESTINATION lib/systemd/system)
install(FILES org.lingmo.locales.policy DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/polkit-1/actions)
