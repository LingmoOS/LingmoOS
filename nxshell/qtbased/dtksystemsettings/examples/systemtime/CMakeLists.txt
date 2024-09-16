# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

add_executable(systemtimedemo
    dsystemtimedemo.cpp
)

target_link_libraries(systemtimedemo PRIVATE
    ${LIB_NAME}
)
