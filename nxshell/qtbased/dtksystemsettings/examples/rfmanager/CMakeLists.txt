# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

add_executable(rfdemo
    main.cpp
    demo.h
    demo.cpp
)

target_link_libraries(rfdemo PRIVATE
    ${LIB_NAME}
)
