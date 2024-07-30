# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

add_executable(drkonqi-sentry-postman main.cpp)
target_link_libraries(drkonqi-sentry-postman
    DrKonqiSentryInternal
)

install(TARGETS drkonqi-sentry-postman DESTINATION ${KDE_INSTALL_LIBEXECDIR})

configure_file(drkonqi-sentry-postman.service.cmake ${CMAKE_CURRENT_BINARY_DIR}/drkonqi-sentry-postman.service)
install(
    FILES drkonqi-sentry-postman.timer drkonqi-sentry-postman.path ${CMAKE_CURRENT_BINARY_DIR}/drkonqi-sentry-postman.service
    DESTINATION ${KDE_INSTALL_SYSTEMDUSERUNITDIR}
)
install(CODE "
    include(${CMAKE_SOURCE_DIR}/cmake/SystemctlEnable.cmake)
    systemctl_enable(drkonqi-sentry-postman.path default.target ${KDE_INSTALL_FULL_SYSTEMDUSERUNITDIR})
    systemctl_enable(drkonqi-sentry-postman.path lingmo-core.target ${KDE_INSTALL_FULL_SYSTEMDUSERUNITDIR})
    systemctl_enable(drkonqi-sentry-postman.timer timers.target ${KDE_INSTALL_FULL_SYSTEMDUSERUNITDIR})
    systemctl_enable(drkonqi-sentry-postman.timer lingmo-core.target ${KDE_INSTALL_FULL_SYSTEMDUSERUNITDIR})
")
