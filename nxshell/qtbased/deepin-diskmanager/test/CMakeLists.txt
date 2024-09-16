
# 设置测试程序名字
set(PROJECT_NAME_TEST
    ${PROJECT_NAME}_test)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)


#include(../cmake/googletest.cmake)
#fetch_googletest(
#    ${PROJECT_SOURCE_DIR}/cmake
#    ${PROJECT_BINARY_DIR}/googletest
#    )

#src files
AUX_SOURCE_DIRECTORY(. SRC_LIST)
AUX_SOURCE_DIRECTORY(./ MAIN)
LIST(APPEND SRC_LIST ${MAIN})
AUX_SOURCE_DIRECTORY(./ut_diskoperation UT_DISKOPERATION)
LIST(APPEND SRC_LIST ${UT_DISKOPERATION})

file(GLOB ALL_HEADERS "../service/diskoperation/*.h" "../service/diskoperation/filesystems/*.h")
file(GLOB ALL_SOURCES "../service/diskoperation/*.cpp" "../service/diskoperation/filesystems/*.cpp")

add_executable(${PROJECT_NAME_TEST} ${SRC_LIST} ${ALL_HEADERS} ${ALL_SOURCES})

target_link_libraries(${PROJECT_NAME_TEST} gmock gmock_main gtest gtest_main pthread Qt5::Core basestruct parted parted-fs-resize)

# 添加 QTest 测试
add_test(${PROJECT_NAME_TEST} that-test-I-made COMMAND ${PROJECT_NAME_TEST})


#------------------------------ 创建'make test'指令---------------------------------------
add_custom_target(test
    COMMAND mkdir -p coverageResult
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

add_custom_command(TARGET test
    COMMAND echo " =================== CREAT LCOV REPROT BEGIN ==================== "

    #1.清理原先的gcov信息
    COMMAND lcov --directory ./CMakeFiles/${PROJECT_NAME_TEST}.dir --zerocounters
    COMMAND lcov --directory ./CMakeFiles/${PROJECT_NAME}.dir --zerocounters
     COMMAND echo " =================== CREAT 1111111 ==================== "
    COMMAND ${PROJECT_BINARY_DIR}/test/${PROJECT_NAME_TEST}
    COMMAND echo " =================== CREAT 2222222 ==================== "

    #2.收集gcov信息到.info文件中
    COMMAND lcov --directory . --capture --output-file ./coverageResult/${PROJECT_NAME}_Coverage.info
    COMMAND echo " =================== CREAT 33333333333 ==================== "

    #3.过滤一些我们不感兴趣的文件的覆盖率信息
    COMMAND echo " =================== do filter begin ==================== "
    COMMAND lcov --remove ./coverageResult/${PROJECT_NAME}_Coverage.info
    '*/${PROJECT_NAME_TEST}_autogen/*' '*/${PROJECT_NAME}_autogen/*' '*/usr/include/*' '*/tests/*' '*/googletest/*'
    -o ./coverageResult/${PROJECT_NAME}_Coverage.info
    COMMAND echo " =================== do filter end ==================== "

    #3.将.info信息生成报告到reprot文件夹中
    COMMAND genhtml -o ./coverageResult/report ./coverageResult/${PROJECT_NAME}_Coverage.info

    COMMAND echo " =================== CREAT LCOV REPROT END ==================== "

    COMMAND echo " Coverage files have been output to ${CMAKE_BINARY_DIR}/coverageResult/report "

    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

#'make test'命令依赖与我们的测试程序
add_dependencies(test ${PROJECT_NAME_TEST})

include_directories(${PROJECT_SOURCE_DIR}/log)
include_directories(${PROJECT_SOURCE_DIR}/basestruct)
include_directories(${PROJECT_SOURCE_DIR}/service)
include_directories(${PROJECT_SOURCE_DIR}/service/diskoperation)



#target_link_libraries(Qt5::Core)


