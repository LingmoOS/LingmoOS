# SPDX-License-Identifier: BSD-2-Clause

add_executable(drkonqi-polkit-helper main.cpp)
target_link_libraries(drkonqi-polkit-helper Qt6::DBus PolkitQt6-1::Core drkonqi-coredumpexcavator)
install(TARGETS drkonqi-polkit-helper DESTINATION ${KDE_INSTALL_LIBEXECDIR_KF})

install(FILES org.kde.drkonqi.conf DESTINATION ${KDE_INSTALL_DBUSDIR}/system.d)
install(FILES org.kde.drkonqi.policy DESTINATION ${KDE_INSTALL_DATADIR}/polkit-1/actions)
configure_file(org.kde.drkonqi.service.in ${CMAKE_CURRENT_BINARY_DIR}/org.kde.drkonqi.service)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.drkonqi.service DESTINATION ${KDE_INSTALL_DBUSSYSTEMSERVICEDIR})
