# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

file(GLOB_RECURSE LOGIN_DEMO_SOURCES "./*.h" "./*.cpp")

add_executable(logindemo
    ${LOGIN_DEMO_SOURCES}
)

target_link_libraries(logindemo PRIVATE
    Qt${QT_VERSION_MAJOR}::Core    
    ${LIB_NAME}
)
