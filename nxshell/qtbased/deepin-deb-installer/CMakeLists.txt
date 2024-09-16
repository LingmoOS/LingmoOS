# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

cmake_minimum_required(VERSION 3.13)

if (NOT DEFINED VERSION)
    set(VERSION 5.3.9)
endif ()

#安全测试的开关
set(CMAKE_SAFETYTEST OFF)

project(deepin_deb_installer)
option(DMAN_RELEAE OFF "Install dman resources to system or not")

find_package(Qt5LinguistTools REQUIRED)
set(POLICY_FILE com.deepin.pkexec.aptInstallDepend.policy)
add_subdirectory(src/AptInstallDepend)
add_subdirectory(src/deb-installer)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_subdirectory(tests)
endif()
add_subdirectory(src/deepin-deb-installer-dev)
add_subdirectory(translations)
#add_subdirectory(tests/FuzzyTest)

