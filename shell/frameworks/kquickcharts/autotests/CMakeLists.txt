# SPDX-FileCopyrightText: 2019,2020 Arjen Hiemstra <ahiemstra@heimr.nl>
# SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
# SPDX-License-Identifier: BSD-2-Clause
find_package(Qt6 COMPONENTS QuickTest)

include(ECMAddTests)

include_directories(${CMAKE_SOURCE_DIR}/src)

ecm_add_tests(
    ArraySourceTest.cpp
    MapProxySourceTest.cpp
    HistoryProxySourceTest.cpp
    ItemBuilderTest.cpp
    LINK_LIBRARIES Qt6::Test QuickCharts
)

add_executable(qmltest qmltest.cpp)
target_link_libraries(qmltest Qt6::QuickTest QuickCharts)

macro(add_qml_tests)
    if (WIN32)
        set(_extra_args -platform offscreen)
    endif()

    foreach(test ${ARGV})
        add_test(NAME ${test}
                 COMMAND ${CMAKE_BINARY_DIR}/bin/qmltest
                         ${_extra_args}
                         -import ${CMAKE_BINARY_DIR}/bin
                         -input ${test}
                 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
    endforeach()
endmacro()

add_qml_tests(
    tst_BarChart.qml
    tst_LineChart.qml
    tst_PieChart.qml
)
