cmake_minimum_required(VERSION 3.0.0)

project(image-editor-test VERSION 0.1)

ADD_COMPILE_OPTIONS(-fno-access-control)

#"option"用来定义宏，"ON"表示打开，"OFF"表示关闭
option (LITE_DIV "Use tutorial provided math implementation" ON)
add_definitions( -DLITE_DIV )

option (USE_TEST "Use TESTS" ON)
add_definitions( -DUSE_TEST )

ADD_COMPILE_OPTIONS(-fno-access-control)

set(TARGET_NAME image-editor-test)
set(CMD_NAME image-editor-test)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
#ADD_DEFINITIONS( -LITE_DIV)
execute_process(COMMAND uname -m OUTPUT_VARIABLE MACH
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)



if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 ")
endif()

# Sources files
#cmakedefine USE_MYMATH
set(AllTestQRC
    ./testqrc/testresource.qrc)
file(GLOB_RECURSE HEADERSCURRENT "*.h")
file(GLOB_RECURSE  SOURCESCURRENT  "*.cpp")

file (GLOB_RECURSE SOURCES
    "../libimageviewer/*.cpp"
    "../libimageviewer/*.h"
    "../libimagevisualresult/exposure.cpp"
    "../libimagevisualresult/exposure.h"
    )
file(GLOB_RECURSE SOURCESC "../libimageviewer/*.c")
#file(GLOB_RECURSE HEADERS "../src/src/module/modulepanel.h")
file(GLOB_RECURSE FORMS "../libimageviewer/*.ui")
file(GLOB_RECURSE RESOURCES "../*.qrc")
# 三方库 tiff-tools tiffcp.c
file(GLOB tiffToolsSources ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/tiff-tools/*.c
                           ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/tiff-tools/*.h)

include_directories(${CMAKE_INCLUDE_CURRENT_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(PROJECT_INCLUDE ${PROJECT_SOURCE_DIR}/../libimageviewer/
    ${PROJECT_SOURCE_DIR}/../src/src/utils
    )

find_package(PkgConfig REQUIRED)
find_package(TIFF)

find_package(Qt5Core)
find_package(Qt5Test)
find_package(Qt5Gui)
find_package(Qt5Widgets)
find_package(Qt5Sql)
find_package(Qt5DBus)
find_package(Qt5Concurrent)
find_package(Qt5Svg)
find_package(Qt5X11Extras)
find_package(Qt5PrintSupport)
find_package(Qt5OpenGL)
find_package(Qt5LinguistTools)

pkg_check_modules(3rd_lib REQUIRED
    dtkcore
    dtkwidget
    gio-qt
    gio-unix-2.0
    gsettings-qt
    gstreamer-1.0
    libmediainfo
        )

## translations

file(GLOB TS LIST_DIRECTORIES false translations/${CMD_NAME}*.ts)
#file(GLOB TS LIST_DIRECTORIES false ${PROJECT_SOURCE_DIR}/translations/image-editor_zh_CN.ts)

set_source_files_properties(${TS} PROPERTIES OUTPUT_LOCATION ${PROJECT_SOURCE_DIR}/translations)

qt5_create_translation(QM ${SOURCES}  ${GLOB})

if (NOT (${CMAKE_BUILD_TYPE} MATCHES "Debug"))
    qt5_create_translation(QM ${SRCS} ${TS})
endif ()

add_executable( ${TARGET_NAME} ${SOURCES} ${SOURCESC}
    ${HEADERS} ${AllTestQRC}
    ${RESOURCES} ${HEADERSCURRENT}
    ${SOURCESCURRENT} ${QM} ${tiffToolsSources}
    )

# 暂未使用OpenCV, 暂时屏蔽
# find_package(OpenCV REQUIRED)
# include_directories(${OpenCV_INCLUDE_DIRS})
# link_directories(${OpenCV_LIBRARY_DIRS})
# target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

target_include_directories(${TARGET_NAME} PUBLIC ${3rd_lib_INCLUDE_DIRS} ${PROJECT_INCLUDE} ${GLOB_RECURSE} ${TIFF_INCLUDE_DIRS})
target_link_libraries(${TARGET_NAME} ${3rd_lib_LIBRARIES} -ldl ${TIFF_LIBRARIES})


target_link_libraries(
    ${PROJECT_NAME}
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Sql
    Qt5::Svg
    Qt5::DBus
    Qt5::Concurrent
    Qt5::X11Extras
    Qt5::PrintSupport
    Qt5::OpenGL
    Qt5::Test
    dtkwidget
    dtkcore
    X11
    Xext
    exif
#    gio-unix-2.0
    gio-qt
    udisks2-qt5
    pthread
    gtest
    )


include_directories(${PROJECT_BINARY_DIR})
include_directories(${PROJECT_SOURCE_DIR})
#configure_file(${PROJECT_SOURCE_DIR}/config.h.in  @ONLY)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)


# 添加gocv覆盖率文件的输出
#set(CMAKE_CXX_FLAGS "-g -fprofile-arcs -ftest-coverage")
# 加速编译优化参数
if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "mips64")
    set(CMAKE_CXX_FLAGS "-g -fprofile-arcs -ftest-coverage")
else()
    set(CMAKE_CXX_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer -g -fprofile-arcs -ftest-coverage -pthread ")
    set(CMAKE_C_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer -pthread ")
    set(CMAKE_Ｌ_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer")
endif()

set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)

set(OBJECT_DIR ${CMAKE_BINARY_DIR}/CMakeFiles/objects)

add_custom_target(test
    COMMAND mkdir -p coverageResult
    COMMAND mkdir -p tests/test
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

add_custom_command(TARGET test
    COMMAND echo " ===================CREAT LCOV REPROT==================== "
    COMMAND lcov --directory ./CMakeFiles/${PROJECT_NAME}.dir --zerocounters
    COMMAND ./tests/${PROJECT_NAME}
    COMMAND lcov --directory . --capture --output-file ./coverageResult/${PROJECT_NAME}_Coverage.info

    COMMAND echo " =================== do filter begin ==================== "
    COMMAND lcov --remove ./coverageResult/${PROJECT_NAME}_Coverage.info 'CMakeFiles/${PROJECT_NAME}.dir/${PROJECT_NAME}_autogen/*/*'
    '${PROJECT_NAME}_autogen/*/*' 'googletest/*/*' '*/usr/include/*' '*/tests/*' '/usr/local/*'
    -o ./coverageResult/${PROJECT_NAME}_Coverage_fileter.info
    COMMAND echo " =================== do filter end ==================== "

    COMMAND genhtml -o ./coverageResult/report ./coverageResult/${PROJECT_NAME}_Coverage_fileter.info
    COMMAND echo " -- Coverage files have been output to ${CMAKE_BINARY_DIR}/coverageResult/report "
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
add_dependencies(test ${PROJECT_NAME})



