# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

add_executable(accountsdemo
    main.cpp
    demo.h
    demo.cpp
)

target_link_libraries(accountsdemo PRIVATE
    ${LIB_NAME}
)
