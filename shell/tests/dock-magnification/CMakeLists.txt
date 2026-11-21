cmake_minimum_required(VERSION 3.22)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(dock_magnification_tests LANGUAGES CXX)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    find_package(LingmoSDK REQUIRED)
    enable_testing()
endif()

set(DOCK_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../src)
add_executable(dock-magnification-test
    magnification_test.cpp
    ${DOCK_SOURCE_DIR}/shell/dock/dock_magnification.cpp
    ${DOCK_SOURCE_DIR}/shell/dock/dock_presentation.cpp
)
target_include_directories(dock-magnification-test PRIVATE ${DOCK_SOURCE_DIR})
target_compile_options(dock-magnification-test PRIVATE -include${DOCK_SOURCE_DIR}/namespace_compat.h)
target_link_libraries(dock-magnification-test PRIVATE Lingmo::SDK)
foreach(case normal-layout storage-order drop-and-reorder overlaps return-to-rest)
    add_test(NAME dock-magnification-${case} COMMAND dock-magnification-test ${case})
endforeach()
