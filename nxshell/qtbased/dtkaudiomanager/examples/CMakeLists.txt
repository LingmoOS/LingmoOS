# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

include_directories(../include)
file(GLOB_RECURSE SRCS "*.h" "*.cpp")

set(EXECUTABLE_OUTPUT_PATH ../)
add_executable(dtkaudiomanager-example
    ${SRCS}
)

target_link_libraries(dtkaudiomanager-example PRIVATE
    ${BIN_NAME}
    Qt${QT_VERSION_MAJOR}::Core
)

