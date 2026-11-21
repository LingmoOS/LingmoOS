# SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
# SPDX-FileCopyrightText: 2026 LingmoOS Contributors
# SPDX-License-Identifier: GPL-3.0-or-later

project(lingmo-appruntime)
set(TARGET lingmo-appruntime)

pkg_check_modules(Systemd REQUIRED IMPORTED_TARGET libsystemd)
find_package(Qt6 REQUIRED COMPONENTS Core)

set(SOURCES
    applicationscope.cpp
    applicationscope.h
    appruntime.cpp
    appruntime.h
    chromiumswitches.cpp
    chromiumswitches.h
    main.cpp
)

add_executable(${TARGET} ${SOURCES})
# The sources name their Qt objects with QString; qt_interop.h converts at the
# toolkit's std::string boundary. Force-included so it is in scope everywhere.
# One token: CMake drops a repeated '-include' option but keeps its argument,
# which would then be compiled as a second source file.
target_compile_options(${TARGET} PRIVATE
    -include${CMAKE_SOURCE_DIR}/qt_interop.h
)
target_link_libraries(${TARGET}
    lingmo-core-common
    Lingmo::SDKBase
    Qt6::Core
    PkgConfig::Systemd
)

install(TARGETS ${TARGET} DESTINATION ${CMAKE_INSTALL_BINDIR})
