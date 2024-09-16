# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

# 设置测试程序名字
set(PROJECT_NAME_TEST
    ${PROJECT_NAME}_test)

#开启moc编译
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

#可访问private
ADD_COMPILE_OPTIONS(-fno-access-control)
ADD_COMPILE_OPTIONS(-fno-inline)
#添加-fno-inline编译选项，禁止内联，能获取到函数地址，可以对内联函数进行打桩
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-inline")

set(CMAKE_SAFETYTEST "${CMAKE_SAFETYTEST_ARG}")
if(CMAKE_SAFETYTEST STREQUAL "")
    set(CMAKE_SAFETYTEST "CMAKE_SAFETYTEST_ARG_OFF")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D${CMAKE_SAFETYTEST}")
if(CMAKE_SAFETYTEST STREQUAL "CMAKE_SAFETYTEST_ARG_ON")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=undefined,address -O2")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=undefined,address -O2")
endif()

# 添加gocv覆盖率文件的输出
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fprofile-arcs -ftest-coverage")

#test files
AUX_SOURCE_DIRECTORY(. SRC_LIST)
AUX_SOURCE_DIRECTORY(./ MAIN)
LIST(APPEND SRC_LIST ${MAIN})
AUX_SOURCE_DIRECTORY(./aria2 ARIA2)
LIST(APPEND SRC_LIST ${ARIA2})
AUX_SOURCE_DIRECTORY(./database DATABASE)
LIST(APPEND SRC_LIST ${DATABASE})
AUX_SOURCE_DIRECTORY(./log LOG)
LIST(APPEND SRC_LIST ${LOG})
AUX_SOURCE_DIRECTORY(./ui UI)
LIST(APPEND SRC_LIST ${UI})
AUX_SOURCE_DIRECTORY(./ui UI)
LIST(APPEND SRC_LIST ${UI})
AUX_SOURCE_DIRECTORY(./ui/createTask CREATETASK)
LIST(APPEND SRC_LIST ${CREATETASK})
AUX_SOURCE_DIRECTORY(./ui/mainFrame MAINFRAME)
LIST(APPEND SRC_LIST ${MAINFRAME})
AUX_SOURCE_DIRECTORY(./ui/settings SETTINGS)
LIST(APPEND SRC_LIST ${SETTINGS})
AUX_SOURCE_DIRECTORY(./ui/websocket WEBSOCKET)
LIST(APPEND SRC_LIST ${WEBSOCKET})

#源头文件
file(GLOB ALL_HEADERS "./*.h"
                      "../src/include/ui/*.h"
                      "../src/include/aria2/*.h"
                      "../src/include/database/*.h"
                      "../src/include/log/*.h"
                      "../src/include/ui/createTask/*.h"
                      "../src/include/ui/mainFrame/*.h"
                      "../src/include/ui/settings/*.h"
                      "../src/include/ui/websocket/*.h"
                      )
#源文件
file(GLOB ALL_SOURCES "../src/src/ui/*.cpp"
                      "../src/src/aria2/*.cpp"
                      "../src/src/database/*.cpp"
                      "../src/src/log/*.cpp"
                      "../src/src/ui/createTask/*.cpp"
                      "../src/src/ui/mainFrame/*.cpp"
                      "../src/src/ui/settings/*.cpp"
                      "../src/src/ui/websocket/*.cpp"
                      )

#添加资源文件
file(GLOB SRC_QRC ${PROJECT_SOURCE_DIR}/src/src/ui/resource/*.qrc)

#需要生成的moc文件，输出文件名称放在变量 mocfiles中，必须在find QT5 package才能调用
QT5_WRAP_CPP (UDLC_UI_HEADERS_MOC ${ALL_HEADERS})

#将test文件，源文件，源头文件，moc文件加入到test程序中
add_executable(${PROJECT_NAME_TEST} ${SRC_LIST} ${ALL_HEADERS} ${ALL_SOURCES} ${UDLC_UI_HEADERS_MOC} ${SRC_QRC})

#添加gtest库，Qt库，dtk库
target_link_libraries(${PROJECT_NAME_TEST} gmock gmock_main gtest gtest_main pthread
    Qt5::Core Qt5::Network Qt5::Widgets Qt5::Svg Qt5::DBus Qt5::Sql Qt5::Test ${DtkWidget_LIBRARIES} ${DtkGui_LIBRARIES} ${DtkCore_LIBRARIES} Qt5::WebChannel Qt5::WebSockets)

# 添加 QTest 测试
add_test(${PROJECT_NAME_TEST} COMMAND ${PROJECT_NAME_TEST})


#------------------------------ 创建'make test'指令---------------------------------------
add_custom_target(test
    COMMAND echo " =================== TEST BEGIN ==================== "
    COMMAND mkdir -p coverageResult
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_BINARY_DIR}/bin/${PROJECT_NAME_TEST}
    COMMAND echo " =================== TEST END ==================== "
)
#add_custom_command(TARGET test
#COMMAND echo " =================== CREAT LCOV REPROT BEGIN ==================== "
# #1.清理原先的gcov信息
#COMMAND lcov --directory ./CMakeFiles/${PROJECT_NAME_TEST}.dir --zerocounters
#COMMAND lcov --directory ./CMakeFiles/${PROJECT_NAME}.dir --zerocounters
# COMMAND ./bin/${PROJECT_NAME_TEST}
# #2.收集gcov信息到.info文件中
#COMMAND lcov --directory . --capture --output-file ./coverageResult/${PROJECT_NAME}_Coverage.info
# #3.过滤一些我们不感兴趣的文件的覆盖率信息
#COMMAND echo " =================== do filter begin ==================== "
#COMMAND lcov --remove ./coverageResult/${PROJECT_NAME}_Coverage.info
#'*/${PROJECT_NAME_TEST}_autogen/*' '*/${PROJECT_NAME}_autogen/*' '*/usr/include/*' '*/tests/*' '*/googletest/*' '${PROJECT_SOURCE_DIR}/test/*' '${PROJECT_SOURCE_DIR}/src/include/*' '${PROJECT_BINARY_DIR}/*'
#'${PROJECT_SOURCE_DIR}/src/src/ui/createTask/*delegate.cpp'
#-o ./coverageResult/${PROJECT_NAME}_Coverage.info
#COMMAND echo " =================== do filter end ==================== "
# #3.将.info信息生成报告到reprot文件夹中
#COMMAND genhtml -o ./coverageResult/report ./coverageResult/${PROJECT_NAME}_Coverage.info
#COMMAND echo " =================== CREAT LCOV REPROT END ==================== "
#COMMAND echo " Coverage files have been output to ${CMAKE_BINARY_DIR}/coverageResult/report "
#WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#)

#'make test'命令依赖与我们的测试程序
add_dependencies(test ${PROJECT_NAME_TEST})

#添加头文件
include_directories(${PROJECT_SOURCE_DIR}/src/include/database)
include_directories(${PROJECT_SOURCE_DIR}/src/include/aria2)
include_directories(${PROJECT_SOURCE_DIR}/src/include/ui)
include_directories(${PROJECT_SOURCE_DIR}/src/include/ui/settings)
include_directories(${PROJECT_SOURCE_DIR}/src/include/ui/mainFrame)
include_directories(${PROJECT_SOURCE_DIR}/src/include/ui/createTask)
include_directories(${PROJECT_SOURCE_DIR}/src/include/ui/websocket)
include_directories(${PROJECT_SOURCE_DIR}/src/include/log)
include_directories(${PROJECT_BINARY_DIR})
