# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2019-2022 Harald Sitter <sitter@kde.org>

add_executable(drkonqi-coredump-launcher main.cpp)
target_link_libraries(drkonqi-coredump-launcher Qt::Core KF6::ConfigCore drkonqi-coredump)
target_compile_options(
    drkonqi-coredump-launcher
    PRIVATE
        -DKDE_INSTALL_FULL_LIBEXECDIR=\"${KDE_INSTALL_FULL_LIBEXECDIR}\"
        -DKDE_KDE_INSTALL_PLUGINDIR=\"${KDE_INSTALL_PLUGINDIR}\"
)
install(TARGETS drkonqi-coredump-launcher DESTINATION ${KDE_INSTALL_LIBEXECDIR})

configure_file(drkonqi-coredump-launcher@.service.cmake ${CMAKE_CURRENT_BINARY_DIR}/drkonqi-coredump-launcher@.service)
install(
    FILES drkonqi-coredump-launcher.socket ${CMAKE_CURRENT_BINARY_DIR}/drkonqi-coredump-launcher@.service
    DESTINATION ${KDE_INSTALL_SYSTEMDUSERUNITDIR}
)
install(CODE "
    include(${CMAKE_SOURCE_DIR}/cmake/SystemctlEnable.cmake)
    systemctl_enable(drkonqi-coredump-launcher.socket sockets.target ${KDE_INSTALL_FULL_SYSTEMDUSERUNITDIR})
")

# The reason this is a plugin is so the main exe doesn't need to link against kio and knotifications needlessly.
add_library(KDECoredumpNotifierTruck MODULE NotifierTruck.cpp NotifierTruck.h)
target_link_libraries(KDECoredumpNotifierTruck Qt::Core KF6::Notifications KF6::KIOGui drkonqi-coredump)
install(TARGETS KDECoredumpNotifierTruck DESTINATION ${KDE_INSTALL_PLUGINDIR}/drkonqi/)
