# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

add_executable(drkonqi-sentry-data main.cpp)
target_link_libraries(drkonqi-sentry-data
    Qt::Core
    Qt::DBus
    KF6::CoreAddons
)
install(TARGETS drkonqi-sentry-data DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
