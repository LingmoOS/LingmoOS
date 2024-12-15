# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

add_executable(kbbacklightdemo
    dkbbacklightdemo.cpp
)

target_link_libraries(kbbacklightdemo PRIVATE
    ${LIB_NAME}
)

add_executable(powerdevicedemo
    dpowerdevicedemo.cpp
)

target_link_libraries(powerdevicedemo PRIVATE
    ${LIB_NAME}
)

add_executable(powersettingsdemo
    dpowersettingsdemo.cpp
)

target_link_libraries(powersettingsdemo PRIVATE
    ${LIB_NAME}
)
