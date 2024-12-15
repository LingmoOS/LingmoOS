# SPDX-FileCopyrightText: 2022-2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

set(BIN_NAME monitor)
add_executable(${BIN_NAME} devicemonitor.cpp)
target_link_libraries(${BIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    ${LIB_NAME}
)
