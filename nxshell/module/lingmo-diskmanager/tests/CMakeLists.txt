
# 设置测试程序名字
set(PROJECT_NAME_TEST
    ${PROJECT_NAME}_app_test)

set(CMAKE_CXX_STANDARD 14)
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
  #安全测试选项
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=undefined,address -O2")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=undefined,address -O2")
endif()

## 添加gocv覆盖率文件的输出
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fprofile-arcs -ftest-coverage")

#src files
AUX_SOURCE_DIRECTORY(. SRC_LIST)
AUX_SOURCE_DIRECTORY(./ MAIN)
LIST(APPEND SRC_LIST ${MAIN})
AUX_SOURCE_DIRECTORY(./ut_application UT_DISKOPERATION)
LIST(APPEND SRC_LIST ${UT_DISKOPERATION})

file(GLOB ALL_HEADERS "../application/*.h" "../application/widgets/*.h" "../application/widgets/customcontrol/*.h" "../application/partedproxy/*.h")
file(GLOB ALL_SOURCES "../application/*.cpp" "../application/widgets/*.cpp" "../application/widgets/customcontrol/*.cpp" "../application/partedproxy/*.cpp")
file(GLOB ALL_QRCS "../application/assets/appicons.qrc")
list(REMOVE_ITEM ALL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../application/main.cpp)

find_package(PkgConfig REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkGui REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
find_package(Qt5 COMPONENTS
    Core
    Widgets
    DBus
    REQUIRED)

set(LINK_LIBS
    Qt5::Core
    Qt5::DBus
    Qt5::Widgets
    Qt5::WidgetsPrivate

    ${DtkWidget_LIBRARIES}
    ${DtkCore_LIBRARIES}
    ${DtkGUI_LIBRARIES}
)

##需要生成的moc文件，输出文件名称放在变量 mocfiles中，必须在find QT5 package才能调用
#QT5_WRAP_CPP (UDLC_UI_HEADERS_MOC ${ALL_HEADERS})

add_executable(${PROJECT_NAME_TEST} ${SRC_LIST} ${ALL_HEADERS} ${ALL_SOURCES} ${ALL_QRCS})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${DtkWidget_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${PROJECT_BINARY_DIR}
)
target_link_libraries(${PROJECT_NAME_TEST} gmock gmock_main gtest gtest_main pthread Qt5::Core basestruct parted parted-fs-resize Qt5::DBus Qt5::Widgets Qt5::WidgetsPrivate Qt5::Test ${LINK_LIBS})

# 添加 QTest 测试
add_test(${PROJECT_NAME_TEST} COMMAND ${PROJECT_NAME_TEST})


#------------------------------ 创建'make test'指令---------------------------------------
add_custom_target(test
#    COMMAND mkdir -p coverageResult_app
#    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

    COMMAND echo " =================== TEST BEGIN ==================== "
    COMMAND mkdir -p coverageResult
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_BINARY_DIR}/tests/${PROJECT_NAME_TEST}
    COMMAND echo " =================== TEST END ==================== "
)

#add_custom_command(TARGET test
#    COMMAND echo " =================== CREAT LCOV REPROT BEGIN ==================== "

#    #1.清理原先的gcov信息
#    COMMAND lcov --directory ./CMakeFiles/${PROJECT_NAME_TEST}.dir --zerocounters
#    COMMAND lcov --directory ./CMakeFiles/${PROJECT_NAME}.dir --zerocounters
#     COMMAND echo " =================== CREAT 1111111 ==================== "
#    COMMAND ${PROJECT_BINARY_DIR}/apptest/${PROJECT_NAME_TEST}
#    COMMAND echo " =================== CREAT 2222222 ==================== "

#    #2.收集gcov信息到.info文件中
#    COMMAND lcov --directory . --capture --output-file ./coverageResult_app/${PROJECT_NAME}_Coverage.info
#    COMMAND echo " =================== CREAT 33333333333 ==================== "

#    #3.过滤一些我们不感兴趣的文件的覆盖率信息
#    COMMAND echo " =================== do filter begin ==================== "
#    COMMAND lcov --remove ./coverageResult_app/${PROJECT_NAME}_Coverage.info
#    '*/${PROJECT_NAME_TEST}_autogen/*' '*/${PROJECT_NAME}_autogen/*' '*/usr/include/*' '*/tests/*' '*/googletest/*' '${PROJECT_SOURCE_DIR}/application/widgets/customcontrol/*delegate.cpp'
#    '${PROJECT_SOURCE_DIR}/application/widgets/customcontrol/sizeinfowidget.cpp' '${PROJECT_SOURCE_DIR}/application/widgets/customcontrol/dmframewidget.cpp'
#    -o ./coverageResult_app/${PROJECT_NAME}_Coverage.info
#    COMMAND echo " =================== do filter end ==================== "

#    #3.将.info信息生成报告到reprot文件夹中
#    COMMAND genhtml -o ./coverageResult_app/report ./coverageResult_app/${PROJECT_NAME}_Coverage.info

#    COMMAND echo " =================== CREAT LCOV REPROT END ==================== "

#    COMMAND echo " Coverage files have been output to ${CMAKE_BINARY_DIR}/coverageResult_app/report "

#    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#    )

#'make test'命令依赖与我们的测试程序
add_dependencies(test ${PROJECT_NAME_TEST})

#include_directories(${PROJECT_SOURCE_DIR}/log)
include_directories(${PROJECT_SOURCE_DIR}/basestruct)
include_directories(${PROJECT_SOURCE_DIR}/application)
include_directories(${PROJECT_SOURCE_DIR}/application/widgets)
include_directories(${PROJECT_SOURCE_DIR}/application/widgets/customcontrol)
include_directories(${PROJECT_SOURCE_DIR}/application/partedproxy)
include_directories(${PROJECT_BINARY_DIR}/application)



#target_link_libraries(Qt5::Core)


