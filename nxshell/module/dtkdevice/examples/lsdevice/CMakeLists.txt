# SPDX-FileCopyrightText: 2022-2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

set(BIN_NAME demo-lsdevice)

file(GLOB_RECURSE SOURCES 
    "./*.h"
    "./*.cpp"
)

add_executable(${BIN_NAME}
    ${SOURCES}
)

target_link_libraries(${BIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    ${LIB_NAME}
    dl
)

target_compile_definitions(${BIN_NAME} PRIVATE PREFIX="${CMAKE_PREFIX_PATH}")
