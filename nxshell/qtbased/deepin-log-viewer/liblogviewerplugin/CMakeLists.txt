#定义需要的cmake版本
cmake_minimum_required(VERSION 3.13)

#设置工程名字
project(log-viewer-Plugin VERSION 0.1.0)
set(TARGET_NAME logviewerplugin)
set(CMD_NAME logviewerplugin)

#设置编译参数
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -ldtkwidget -ldtkgui -ldtkcore")#${EXE_NAME}
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--as-needed -fPIE")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -std=c++11")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie")
#安全测试加固编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -z relro -z now -z noexecstack -pie")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -z relro -z now -z noexecstack -pie")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fstack-protector-all")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -fstack-protector-all")
if(CMAKE_COVERAGE_ARG STREQUAL "CMAKE_COVERAGE_ARG_ON")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -fprofile-arcs -ftest-coverage")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -fprofile-arcs -ftest-coverage")
endif()
#安全测试加固编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -z relro -z now -z noexecstack -pie")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -z relro -z now -z noexecstack -pie")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fstack-protector-all")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -fstack-protector-all")
#-ldtkwidget -ldtkgui -ldtkcore
if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "sw_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
endif ()

#预定义宏
add_compile_definitions (LOGVIEWERPLUGIN_LIBRARY)
add_definitions(-DUSE_POLKIT -DENABLE_INACTIVE_DISPLAY)
add_definitions( -DLITE_DIV )
add_definitions( -DUSE_UNIONIMAGE )
add_definitions( -DCMAKE_BUILD )

#Qt需要的包
set(LINK_LIBS
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    Qt5::Xml
    Qt5::DBus
    Qt5::Concurrent
)

#查找依赖的库
find_package(PkgConfig REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Concurrent REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkGui REQUIRED)
find_package(DtkCore REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
find_package(RapidJSON)
find_package(Boost)
find_package(XercesC)
find_package(ZLIB)
find_package(PolkitQt5-1)

#设置输出目录
#set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../deepin-album/lib)

#库目录
aux_source_directory(logviewerplugin allSources)

#需要打开的头文件
FILE (GLOB allHeaders
    "*.h"
    "*/*.h"
    "*/*/*.h"
    "*/*/*/*.h"
    "../application/dbusproxy/dldbusinterface.h"
    "../application/dbusproxy/dldbushandler.h"
    "../application/exportprogressdlg.h"
    "../application/journalbootwork.h"
    "../application/journalwork.h"
    "../application/logapplicationparsethread.h"
    "../application/logoocfileparsethread.h"
    "../application/logexportthread.h"
    "../application/logauththread.h"
    "../application/logfileparser.h"
    "../application/sharedmemorymanager.h"
    "../application/utils.h"
    "../application/wtmpparse.h"
    "../application/structdef.h"
    )
#需要打开的代码文件
FILE (GLOB allSources
    "*.cpp"                                   "*.c"
    "*/*.cpp"                                 "*/*.c"
    "*/*/*.cpp"                              "*/*/*.c"
    "*/*/*/*.cpp"                           "*/*/*/*.c"
    "../application/dbusproxy/dldbusinterface.cpp"
    "../application/dbusproxy/dldbushandler.cpp"
    "../application/exportprogressdlg.cpp"
    "../application/journalbootwork.cpp"
    "../application/journalwork.cpp"
    "../application/logapplicationparsethread.cpp"
    "../application/logoocfileparsethread.cpp"
    "../application/logexportthread.cpp"
    "../application/logauththread.cpp"
    "../application/logfileparser.cpp"
    "../application/sharedmemorymanager.cpp"
    "../application/utils.cpp"
    "../application/wtmpparse.cpp"
    "../application/wtmpparse.cpp"
    )
file(GLOB_RECURSE RESOURCES "*.qrc")

#第三方库目录
set(INC_DIR /usr/include/)
set(LINK_DIR /usr/lib/)
include_directories(${INC_DIR})
link_directories(${LINK_DIR})

#--------------------libxlsx源码引入---------------------------------
file(GLOB LXW_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/libxlsxwriter/src/*.c)
file(GLOB_RECURSE LXW_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/libxlsxwriter/include *.h)

#set (MINIZIP_SOURCES
#    ../3rdparty/minizip/ioapi.c
##../3rdparty/minizip/miniunz.c
#    ../3rdparty/minizip/minizip.c
#    ../3rdparty/minizip/mztools.c
#    ../3rdparty/minizip/unzip.c
#    ../3rdparty/minizip/zip.c
#    )

#file(GLOB TMPFILE_SOURCES  ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/tmpfileplus/*.c)
#file(GLOB MD5SOURCES   ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/md5/*.c)

#---------------DOCXFACTORY源码引入----------------------
file(GLOB_RECURSE DOCXFAC_SOURCES  "${PROJECT_SOURCE_DIR}/3rdparty/DocxFactory/src/*.cpp" )
file(GLOB_RECURSE DOCXFAC_SOURCES_C  "${PROJECT_SOURCE_DIR}/3rdparty/DocxFactory/src/*.c" )

#设置不删除生成的文件夹内容文件
set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1)

#编译为库
add_library(logviewerplugin SHARED ${allHeaders} ${allSources} ${RESOURCES} ${LXW_SOURCES} ${DOCXFAC_SOURCES} ${DOCXFAC_SOURCES_C} ${QM})
target_compile_options(logviewerplugin PRIVATE ${COMPILE_OPTIONS})
set_target_properties(${TARGET_NAME} PROPERTIES VERSION 0.1.0 SOVERSION 0.1)

#将库安装到指定位置
set(PREFIX /usr)
set(${TARGET_NAME} ${CMAKE_INSTALL_LIBDIR})

#包含目录
include_directories(${CMAKE_INCLUDE_CURRENT_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${DtkWidget_INCLUDE_DIRS})
include_directories(${DtkCore_INCLUDE_DIRS})
include_directories(${DtkGui_INCLUDE_DIRS})
include_directories( ${Qt5Gui_PRIVATE_INCLUDE_DIRS})
include_directories( ${Qt5Widgets_PRIVATE_INCLUDE_DIRS})
include_directories( ${Boost_INCLUDE_DIRS})
include_directories( ${ZLIB_INCLUDE_DIRS})
message("ZLIB_INCLUDE_DIRS ${ZLIB_INCLUDE_DIRS}")
include_directories( ${RapidJSON_INCLUDE_DIRS})
include_directories(${XercesC_INCLUDE_DIRS})
include_sub_directories_recursively("${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/libxlsxwriter")
include_sub_directories_recursively("${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/DocxFactory/include")

#引用库
target_link_libraries(${TARGET_NAME}  ${LINK_LIBS} -lsystemd -licui18n -licuuc  -ldl -fPIC)
target_link_libraries(${TARGET_NAME} ${DtkWidget_LIBRARIES})
target_link_libraries(${TARGET_NAME} ${DtkCore_LIBRARIES})
target_link_libraries(${TARGET_NAME} ${DtkGUI_LIBRARIES})
#当binutils>=2.22时，对于链接的静态库链接的静态库，cmake并不会递归链接，所以需要手动链接libxlsxwriter需要的lzlib
target_link_libraries(${TARGET_NAME} ${ZLIB_LIBRARIES})
target_link_libraries(${TARGET_NAME} ${Boost_LIBRARIES})
target_link_libraries(${TARGET_NAME} ${RapidJSON_LIBRARIES})
target_link_libraries(${TARGET_NAME} ${XercesC_LIBRARIES})
target_link_libraries(${TARGET_NAME} PolkitQt5-1::Agent)

include(GNUInstallDirs)
configure_file(liblogviewerplugin.pc.in ${PROJECT_BINARY_DIR}/liblogviewerplugin.pc @ONLY)

#安装位置
install(TARGETS ${CMD_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES
    ../application/structdef.h
    DESTINATION include/liblogviewerplugin)
install(FILES
    src/plugininterfaces/logviewerplugininterface.h
    DESTINATION include/liblogviewerplugin/plugininterfaces)
install(FILES
    src/plugins/logviewerplugin.h
    DESTINATION include/liblogviewerplugin/plugins)

install(FILES ${PROJECT_BINARY_DIR}/liblogviewerplugin.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

#translations翻译
#file(GLOB TS LIST_DIRECTORIES false translations/liblogviewerplugin*.ts)
#set_source_files_properties(${TS} PROPERTIES OUTPUT_LOCATION ${PROJECT_SOURCE_DIR}/translations)

#qt5_create_translation(QM ${allSources}  ${GLOB})

#if (NOT (${CMAKE_BUILD_TYPE} MATCHES "Debug"))
#    qt5_create_translation(QM ${allSources} ${TS})
#endif ()

#install(DIRECTORY ${PROJECT_SOURCE_DIR}/translations
#    DESTINATION ${PREFIX}/share/liblogviewerplugin
#    FILES_MATCHING PATTERN "*.qm")

