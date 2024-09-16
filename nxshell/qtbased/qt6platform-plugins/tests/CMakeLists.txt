# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

project(ut-platformplugins)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui Widgets Concurrent Test)
find_package(GTest REQUIRED)
if(${QT_VERSION_MAJOR} STREQUAL "5")
    find_package(Qt5 REQUIRED COMPONENTS XcbQpa X11Extras EdidSupport XkbCommonSupport)
else()
    find_package(Qt6 REQUIRED COMPONENTS OpenGL XcbQpaPrivate)
endif()

add_definitions(-DDXCB_VERSION=\"${DTK_VERSION}\")
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DQT_NO_DEBUG_OUTPUT=TRUE)
endif()


file(GLOB test_SRC test.h main.cpp src/*.cpp)

add_executable(${PROJECT_NAME} ${test_SRC})

include(${CMAKE_SOURCE_DIR}/xcb/linux.cmake)

add_definitions(-DPLUGIN_OUTPUT_PATH=\"${LIBRARY_OUTPUT_PATH}/..\")

target_compile_options(${PROJECT_NAME} PRIVATE -fno-access-control -fsanitize=address)
target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(${PROJECT_NAME} PRIVATE -fprofile-instr-generate -ftest-coverage)
endif()
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(${PROJECT_NAME} PRIVATE -fprofile-arcs -ftest-coverage)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/xcb
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::GuiPrivate
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Test
    GTest::GTest
    gmock
    pthread
    m
    gcov
    -ldl
    dxcb
)

if(${QT_VERSION_MAJOR} STREQUAL "5")
    target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt5::XcbQpa
        Qt5::EdidSupport
        Qt5::EdidSupportPrivate
        Qt5::XkbCommonSupport
        Qt5::XkbCommonSupportPrivate
        Qt5::X11Extras
    )
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::OpenGL Qt6::OpenGLPrivate Qt6::XcbQpaPrivate)
endif()
