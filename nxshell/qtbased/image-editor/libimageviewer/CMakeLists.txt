#定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

# 设置工程名字
project(libimageviewer VERSION 0.1.0)

# 检查dtkwidget版本
execute_process(COMMAND dpkg -s libdtkwidget-dev
                COMMAND grep Version
                TIMEOUT 5
                OUTPUT_VARIABLE DWIDGET_VERSION_STR)
string(REPLACE "Version: " "" DWIDGET_VERSION_STR ${DWIDGET_VERSION_STR})
message(STATUS "BUILD DTKWidget VERSION ${DWIDGET_VERSION_STR}")

if (${DWIDGET_VERSION_STR} VERSION_LESS "5.6.9-1")
    message(STATUS "Use specific dtk watermakr, Version < 5.6.9-1")
    add_definitions(-DUSE_SPECIFIC_DTK_WATERMARK)
else()
    message(STATUS "Use master dtk watermakr, Version >= 5.6.9-1")
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_C_FLAGS "-fPIC")
set(CMAKE_CXX_FLAGS "-fPIC")

#Qt需要的包
set(QtModule Core Gui Widgets Svg DBus Concurrent PrintSupport LinguistTools)

set(TARGET_NAME imageviewer)
set(CMD_NAME imageviewer)

#先查找到这些qt相关的模块以供链接使用
find_package(Qt5 REQUIRED ${QtModule})

#设置输出目录
#set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../deepin-album/lib)

#包含目录
include_directories(${CMAKE_INCLUDE_CURRENT_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_definitions( -DLITE_DIV )
add_definitions( -DUSE_UNIONIMAGE )
add_definitions( -DCMAKE_BUILD )

#库目录
aux_source_directory(imageviewer allSources)


find_package(PkgConfig REQUIRED)
find_package(TIFF)
pkg_check_modules(3rd_lib REQUIRED
    dtkwidget
    dtkcore
    dtkgui
    gobject-2.0
    libmediainfo
    )

# 查找 dfm-io
pkg_check_modules(dfm-io_lib dfm-io)
if(${dfm-io_lib_FOUND})
    message("Found dfm-io, enable MTP file copy optimization.")
    add_definitions(-DUSE_DFM_IO)
else()
    message("Not found dfm-io, use base api copy MTP file.")
endif()

#需要打开的头文件
FILE (GLOB allHeaders "*.h" "*/*.h" "*/*/*.h")
#需要打开的代码文件
FILE (GLOB allSources
    "*.cpp"                                   "*.c"
    "*/*.cpp"                                 "*/*.c"
    "*/*/*.cpp"                              "*/*/*.c"
    )
file(GLOB_RECURSE RESOURCES "*.qrc")

#------------------添加第三方库begins--------------------------#
file(GLOB tiffToolsSources ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/tiff-tools/*.c
                           ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/tiff-tools/*.h)
#使用第三方库需要用到的一个包

set(INC_DIR /usr/include/)
set(LINK_DIR /usr/lib/)

include_directories(${INC_DIR})
link_directories(${LINK_DIR})

link_libraries(freeimage)

set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1)

file(GLOB TS LIST_DIRECTORIES false translations/libimageviewer*.ts)

set_source_files_properties(${TS} PROPERTIES OUTPUT_LOCATION ${PROJECT_SOURCE_DIR}/translations)

qt5_create_translation(QM ${allSources}  ${GLOB})

if (NOT (${CMAKE_BUILD_TYPE} MATCHES "Debug"))
    qt5_create_translation(QM ${allSources} ${TS})
endif ()

# 设置不删除生成的文件夹内容文件
set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1)

#编译为库
add_library(imageviewer SHARED ${allHeaders} ${allSources} ${RESOURCES} ${QM} ${tiffToolsSources})
#使用库
qt5_use_modules(imageviewer ${QtModule})


#将库安装到指定位置
set(PREFIX /usr)
set(${TARGET_NAME} ${CMAKE_INSTALL_LIBDIR})

set_target_properties(${TARGET_NAME} PROPERTIES VERSION 0.1.0 SOVERSION 0.1)

target_include_directories(${CMD_NAME} PUBLIC ${3rd_lib_INCLUDE_DIRS}  ${TIFF_INCLUDE_DIRS} ${dfm-io_lib_INCLUDE_DIRS})
target_link_libraries(imageviewer ${3rd_lib_LIBRARIES} freeimage ${TIFF_LIBRARIES} ${dfm-io_lib_LIBRARIES} dl)

include(GNUInstallDirs)
configure_file(libimageviewer.pc.in ${PROJECT_BINARY_DIR}/libimageviewer.pc @ONLY)

install(TARGETS ${CMD_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})

# 公共库头文件
install(FILES
    imageengine.h
    imageviewer.h
    image-viewer_global.h
    movieservice.h
    quickprint.h
    DESTINATION include/libimageviewer)

# pkg-config 配置文件
install(FILES ${PROJECT_BINARY_DIR}/libimageviewer.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
# translations 翻译文件
install(DIRECTORY ${PROJECT_SOURCE_DIR}/translations
    DESTINATION ${PREFIX}/share/libimageviewer
    FILES_MATCHING PATTERN "*.qm")

# 加速编译优化参数
if (NOT(CMAKE_BUILD_TYPE STREQUAL "Debug"))
    if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "mips64")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wl,-O1 -Wl,-E -fPIE -ffunction-sections -fdata-sections -Wl,--gc-sections -ftree-vectorize -march=loongson3a -mhard-float -mno-micromips -mno-mips16 -flax-vector-conversions -mloongson-ext2 -mloongson-mmi -z noexecstack")
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -Wl,-O1 -Wl,-E -fPIE -ffunction-sections -fdata-sections -Wl,--gc-sections -ftree-vectorize -march=loongson3a -mhard-float -mno-micromips -mno-mips16 -flax-vector-conversions -mloongson-ext2 -mloongson-mmi -z noexecstack")
        SET(CMAKE_EXE_LINKER_FLAGS  "-pie")
    else()
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wl,-O1 -Wl,-E -fPIE -ffunction-sections -fdata-sections -Wl,--gc-sections")
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3  -Wl,-O1 -Wl,-E -fPIE -ffunction-sections -fdata-sections -Wl,--gc-sections")
        SET(CMAKE_EXE_LINKER_FLAGS  "-pie")
    endif()
endif()

if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "sw_64")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mieee")
    add_definitions( -DISSW_64 )
endif()
