# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2021-2022 Harald Sitter <sitter@kde.org>

add_library(drkonqi-coredump STATIC coredump.cpp coredumpwatcher.cpp)
target_link_libraries(drkonqi-coredump PUBLIC Qt::Core Qt::Network Systemd::systemd)
set_property(TARGET drkonqi-coredump PROPERTY POSITION_INDEPENDENT_CODE ON)

add_subdirectory(cleanup)
add_subdirectory(processor)
add_subdirectory(launcher)
add_subdirectory(gui)
add_subdirectory(polkit)
add_subdirectory(dumpexcavator)
