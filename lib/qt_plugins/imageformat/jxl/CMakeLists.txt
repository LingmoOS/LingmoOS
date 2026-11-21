# SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
# SPDX-FileCopyrightText: 2026 LingmoOS Contributors
# SPDX-License-Identifier: GPL-3.0-or-later

pkg_check_modules(LIBJXL IMPORTED_TARGET libjxl libjxl_threads)
if(NOT LIBJXL_FOUND)
    message(STATUS "libjxl not found: no JPEG XL image plugin")
    return()
endif()

add_library(qjxl MODULE
    jxlhandler.cpp
    jxlhandler.h
    jxlplugin.cpp
    jxlplugin.h
)
target_compile_definitions(qjxl PRIVATE QT_NO_KEYWORDS)
set_target_properties(qjxl PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/imageformats")
target_link_libraries(qjxl Qt6::Core Qt6::Gui PkgConfig::LIBJXL)
install(TARGETS qjxl DESTINATION ${CMAKE_INSTALL_QTPLUGINDIR}/imageformats/)
