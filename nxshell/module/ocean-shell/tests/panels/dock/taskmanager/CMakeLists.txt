# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: CC0-1.0

find_package(GTest REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS Core Test)

add_executable(rolecombinemodel_tests
    combinemodela.cpp
    combinemodela.h
    combinemodelb.cpp
    combinemodelb.h
    rolecombinemodeltests.cpp
)

target_link_libraries(rolecombinemodel_tests
    GTest::GTest
    GTest::Main
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
    dock-taskmanager
)
target_include_directories(rolecombinemodel_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/panels/dock/taskmanager/
)

add_test(NAME rolecombinemodel COMMAND rolecombinemodel_tests)
