
#版本设置
set(VERSION 0.1)
#二进制名称
set(PROJECT_NAME_TEST
    ${PROJECT_NAME}_test)#项目名称设置

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
set(CMAKE_SAFETYTEST "CMAKE_SAFETYTEST_ARG_ON")
if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64")
    set(CMAKE_SAFETYTEST "${CMAKE_SAFETYTEST_ARG}")
    if(CMAKE_SAFETYTEST STREQUAL "")
        set(CMAKE_SAFETYTEST "CMAKE_SAFETYTEST_ARG_OFF")
    endif()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D${CMAKE_SAFETYTEST}")
    if(CMAKE_SAFETYTEST STREQUAL "CMAKE_SAFETYTEST_ARG_ON")
    #安全测试选项
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=undefined,address -O2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=undefined,address -O2")
    endif()
endif()

if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wextra -fprofile-arcs -ftest-coverage -lgcov")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -pthread -fprofile-arcs -ftest-coverage -lgcov")
  # Enable Qt builtin debug mode
  add_definitions("-DQT_MESSAGELOGCONTEXT -std=c++11")
else()
  # -Wl, -O2 Enable linker optimizations
  # -Wl, --gc-sections Remove unused code resulting from -fdsta-sections and
  # -ffunction-sections
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -fprofile-arcs -ftest-coverage -lgcov -Wl,--gc-sections")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -fprofile-arcs -ftest-coverage -lgcov -Wl,--gc-sections -pthread")
endif()

#可访问private
ADD_COMPILE_OPTIONS(-fno-access-control)
ADD_COMPILE_OPTIONS(-fno-inline)
#添加-fno-inline编译选项，禁止内联，能获取到函数地址，可以对内联函数进行打桩
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-inline")

# 查找库
enable_testing()
find_package(GTest REQUIRED)
find_package(Qt5 COMPONENTS Test REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkGui REQUIRED)
find_package(DtkCore REQUIRED)
find_package(DtkCMake REQUIRED)
#find_package(DdeControlCenter REQUIRED)
find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Gui REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS DBus REQUIRED)
find_package(Qt5 COMPONENTS LinguistTools REQUIRED)
find_package(Qt5 COMPONENTS Concurrent REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Fcitx REQUIRED)
find_package(FcitxQt5DBusAddons REQUIRED)
find_package(FcitxQt5WidgetsAddons REQUIRED)
find_package(KF5 REQUIRED COMPONENTS I18n)
pkg_search_module(GSETTINGS REQUIRED gsettings-qt)
#设置资源目录 加载资源文件 翻译文件 源码
set(APP_RES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../res")
#创建翻译文件 
#set(TS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../translations")
#set(TS_FILES "${TS_DIR}/${PROJECT_NAME}_en_US.ts")
#更新翻译
#find_program(LUPDATE_EXECUTABLE lupdate)
foreach(_ts_file ${TS_FILES})
    execute_process(
        COMMAND ${LUPDATE_EXECUTABLE} -recursive ${CMAKE_SOURCE_DIR} -ts ${_ts_file})  #递归扫描源码 输出ts文件
endforeach()

#创建qm文件
file (GLOB DTNG_TS_FILES ${TS_DIR}/*.ts)
qt5_create_translation(DTNG_QM_FILES
     ${DTNG_TS_FILES}
     ${DTNG_QM_FILES})

FILE(GLOB_RECURSE TEST_HEAD_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

FILE(GLOB_RECURSE TEST_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

FILE(GLOB_RECURSE HEAD_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ddeInterface/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/fcitxInterface/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/widgets/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/window/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/fcitxconfigplugin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/publisher/*.h)

FILE(GLOB_RECURSE SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ddeInterface/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/fcitxInterface/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/widgets/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/window/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/fcitxconfigplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/publisher/*.cpp)

set(ALL_SRCS
    ${HEAD_FILES}
    ${SOURCE_FILES}
    ${APP_QRC}
    ${DTNG_QM_FILES}
    )

set(TEST_SRCS
    ${TEST_HEAD_FILES}
    ${TEST_SOURCE_FILES}
    )

add_executable(${PROJECT_NAME_TEST}
    ${ALL_SRCS}
    ${DTNG_QM_FILES}
    ${TEST_SRCS}
    )

include_directories(${PROJECT_NAME_TEST} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../src)

target_include_directories(${PROJECT_NAME_TEST} PUBLIC
        "/usr/include/dde-control-center"
#${DdeControlCenter_INCLUDE_DIR}
        ${DtkCore_INCLUDE_DIRS}
        ${DtkWidget_INCLUDE_DIRS}
        ${DFrameworkDBus_INCLUDE_DIRS}
        ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
        ${GTEST_INCLUDE_DIR}
        ${GSETTINGS_INCLUDE_DIRS}
        )

target_link_libraries(${PROJECT_NAME_TEST} PRIVATE
        "/usr/lib/dde-control-center"
#${DdeControlCenter_LIBRARIES}
        KF5::I18n
        ${DFrameworkDBus_LIBRARIES}
        ${DtkWidget_LIBRARIES}
        ${Qt5Widgets_LIBRARIES}
        ${Qt5DBus_LIBRARIES}
        ${Qt5Concurrent_LIBRARIES}
        ${OBJECT_BINARY_DIR}
        ${FCITX4_FCITX_UTILS_LIBRARIES}
        ${FCITX4_FCITX_CONFIG_LIBRARIES}
        ${FCITX4_FCITX_LIBRARIES}
        FcitxQt5::WidgetsAddons
        FcitxQt5::DBusAddons
        ${Qt5Test_LIBRARIES}
        ${GTEST_LIBRARIES}
        ${GSETTINGS_LIBRARIES}
        )

#------------------------------ 创建'make test'指令---------------------------------------
add_custom_target(test
    COMMAND mkdir -p coverageResult
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})


#add_custom_command(TARGET test
#    COMMAND echo " =================== CREAT LCOV REPROT BEGIN ==================== "
#
#    #1.清理原先的gcov信息
#    COMMAND lcov --directory ./CMakeFiles/${PROJECT_NAME}_test.dir --zerocounters
#    COMMAND lcov --directory ./CMakeFiles/${PROJECT_NAME}.dir --zerocounters
#    COMMAND ./src/${PROJECT_NAME}_test
#
#    #2.收集gcov信息到.info文件中
#    COMMAND lcov --directory . --capture --output-file ./coverageResult/${PROJECT_NAME}_Coverage.info
#
#    #3.过滤一些我们不感兴趣的文件的覆盖率信息
#    COMMAND echo " =================== do filter begin ==================== "
#    COMMAND lcov --remove ./coverageResult/${PROJECT_NAME}_Coverage.info
#    '*/${PROJECT_NAME}_test_autogen/*' '*/${PROJECT_NAME}_autogen/*' '*/usr/include/*' '*/tests/*' '*/googletest/*' '*/unittest/*' '*/widgets/*' '*/FcitxQtDBusAddons/*' '*/ddeInterface/*' '*/fcitxInterface/*'
#    -o ./coverageResult/${PROJECT_NAME}_Coverage.info
#    COMMAND echo " =================== do filter end ==================== "
#
#    #3.将.info信息生成报告到reprot文件夹中
#    COMMAND genhtml -o ./coverageResult/report ./coverageResult/${PROJECT_NAME}_Coverage.info
#
#    COMMAND echo " =================== CREAT LCOV REPROT END ==================== "
#
#    COMMAND echo " Coverage files have been output to ${CMAKE_BINARY_DIR}/coverageResult/report "
#
#    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#    )
#
#'make test'命令依赖与我们的测试程序
add_dependencies(test ${PROJECT_NAME_TEST})
