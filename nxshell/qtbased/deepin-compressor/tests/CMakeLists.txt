# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

#1.定义cmake版本
cmake_minimum_required(VERSION 3.9.5)
#2.设置项目名称
set(EXE_NAME test)
project(${EXE_NAME})

add_subdirectory(UnitTest)

add_custom_target(test
    COMMAND echo " =================== TEST BEGIN ==================== "
    COMMAND ${CMAKE_BINARY_DIR}/bin/tests/bz2plugin_test
    COMMAND ${CMAKE_BINARY_DIR}/bin/tests/cli7zplugin_test
    COMMAND ${CMAKE_BINARY_DIR}/bin/tests/clirarplugin_test
    COMMAND ${CMAKE_BINARY_DIR}/bin/tests/deepin-compressor_test
    COMMAND ${CMAKE_BINARY_DIR}/bin/tests/gzplugin_test
    COMMAND ${CMAKE_BINARY_DIR}/bin/tests/interface_test
    COMMAND ${CMAKE_BINARY_DIR}/bin/tests/libminizipplugin_test
    COMMAND ${CMAKE_BINARY_DIR}/bin/tests/libpigzplugin_test
    COMMAND ${CMAKE_BINARY_DIR}/bin/tests/libzipplugin_test
    COMMAND ${CMAKE_BINARY_DIR}/bin/tests/readonlylibarchiveplugin_test
    COMMAND ${CMAKE_BINARY_DIR}/bin/tests/readwritelibarchiveplugin_test
    COMMAND ${CMAKE_BINARY_DIR}/bin/tests/xzplugin_test
    COMMAND echo " =================== TEST END ==================== "
)

# debian 会自动执行 make test
#'make test'命令依赖与我们的测试程序
#add_dependencies(test ${PROJECT_NAME_TEST})

