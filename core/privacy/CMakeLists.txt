project(lingmo-privacy)
set(TARGET lingmo-privacy)

set(SOURCES
    captureswitches.cpp
    captureswitches.h
    main.cpp
    privacyservice.cpp
    privacyservice.h
    usbcameras.cpp
    usbcameras.h
)

add_executable(${TARGET} ${SOURCES})
target_link_libraries(${TARGET}
    Lingmo::SDKBase
    lingmo-core-common
)

configure_file(org.lingmo.Privacy.service.in org.lingmo.Privacy.service @ONLY)
configure_file(lingmo-privacy.service.in lingmo-privacy.service @ONLY)

install(TARGETS ${TARGET} DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
install(FILES org.lingmo.Privacy.conf DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/dbus-1/system.d)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.lingmo.Privacy.service
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/dbus-1/system-services)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lingmo-privacy.service DESTINATION lib/systemd/system)
install(FILES org.lingmo.privacy.policy DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/polkit-1/actions)
install(FILES 60-lingmo-privacy.rules DESTINATION lib/udev/rules.d)
