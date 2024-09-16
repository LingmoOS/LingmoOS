#1.定义cmake版本
cmake_minimum_required(VERSION 3.9.5)

#2.设置项目名称
set(EXE_NAME compressor)
project(${EXE_NAME})

if (NOT DEFINED VERSION)
    set(VERSION 1.0)
endif ()

#3.输出目录的预设值
include(GNUInstallDirs)
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
   set(CMAKE_INSTALL_PREFIX /usr)
endif ()

## 插件安装目录
if(NOT DEFINED COMPRESSOR_PLUGIN_PATH)
    set(COMPRESSOR_PLUGIN_PATH ${CMAKE_INSTALL_LIBDIR}/deepin-compressor/plugins)
endif()

## 导出头文件路径,非安装时使用
if(NOT DEFINED HFILES_OUTPUT_PATH)
    set(HFILES_OUTPUT_PATH ${CMAKE_BINARY_DIR}/include)
endif()

#SET(CMAKE_BUILD_TYPE "Debug")  
#SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")


# 添加c++ 11标准支持
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -std=c++11" )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--as-needed -fPIE -fPIC") #-Wl,--as-needed 减少无效的库链接，用不到的库需要清理

if(DEFINED ENV{PREFIX})
    set(CMAKE_INSTALL_PREFIX $ENV{PREFIX})
    set(COMPRESSOR_PLUGIN_PATH lib/deepin-compressor/plugins)
    if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -Wall -g -ggdb -Wl,-rpath=${CMAKE_INSTALL_PREFIX}/lib/deepin-compressor/plugins")
    else()
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wl,-O1,-rpath=${CMAKE_INSTALL_PREFIX}/lib/deepin-compressor/plugins")
    endif()
else()
    if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -Wall -g -ggdb -Wl,-rpath=../3rdparty/interface:../../3rdparty/interface:${COMPRESSOR_PLUGIN_PATH}")
    else()
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wl,-O1,-rpath=${COMPRESSOR_PLUGIN_PATH}")
    endif()
endif()

#安全编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong -D_FORTITY_SOURCE=1 -z noexecstack -pie -fPIC -z lazy")

#代码覆盖率开关
if(CMAKE_COVERAGE_ARG STREQUAL "CMAKE_COVERAGE_ARG_ON")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -fprofile-arcs -ftest-coverage")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -fprofile-arcs -ftest-coverage")
endif()

add_definitions("-DQT_MESSAGELOGCONTEXT")
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE ON) #要显示执行构建过程中详细的信息(比如为了得到更详细的出错信息)
set(CMAKE_AUTOMOC ON) #打开全局moc
set(CMAKE_AUTORCC ON)


#对DApplication 进行加速
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie")

#龙芯特有编译参数 -start
if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "mips64")
    message(STATUS "this system is mips64=========")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ftree-vectorize -march=loongson3a -mhard-float -mno-micromips -mno-mips16 -flax-vector-conversions -mloongson-ext2 -mloongson-mmi -Wl,--as-needed")
endif()
#龙芯特有编译参数 -end

#模糊测试设置 -start
if((${CMAKE_BUILD_TYPE} MATCHES "Debug"))
 #   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
 #   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
  #  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address")
  #  set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -fsanitize=address")
endif()
#模糊测试设置 -end


#Generate the qm file
set(APP_RES_DIR "src")
set(APP_TRANS_SCRIPT "${APP_RES_DIR}/translate_generation.sh")
execute_process(COMMAND bash ${APP_TRANS_SCRIPT}
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/")

add_subdirectory(3rdparty)
add_subdirectory(src)

#单元测试开关
if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    option(USE_TEST "UT" ON)
else()
    option(USE_TEST "UT" OFF)
endif()
if (USE_TEST)
    add_subdirectory(tests)
endif(USE_TEST)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in ${HFILES_OUTPUT_PATH}/config.h @ONLY)


install(TARGETS cli7zplugin DESTINATION ${COMPRESSOR_PLUGIN_PATH})
install(TARGETS clirarplugin DESTINATION ${COMPRESSOR_PLUGIN_PATH})
install(TARGETS libzipplugin DESTINATION ${COMPRESSOR_PLUGIN_PATH})
install(TARGETS libpigzplugin DESTINATION ${COMPRESSOR_PLUGIN_PATH})
install(TARGETS minizipplugin DESTINATION ${COMPRESSOR_PLUGIN_PATH})
install(TARGETS readwritelibarchiveplugin DESTINATION ${COMPRESSOR_PLUGIN_PATH})
install(TARGETS readonlylibarchiveplugin DESTINATION ${COMPRESSOR_PLUGIN_PATH})
install(TARGETS compressor-interface DESTINATION ${COMPRESSOR_PLUGIN_PATH})
install(TARGETS bz2plugin DESTINATION ${COMPRESSOR_PLUGIN_PATH})
install(TARGETS gzplugin DESTINATION ${COMPRESSOR_PLUGIN_PATH})
install(TARGETS xzplugin DESTINATION ${COMPRESSOR_PLUGIN_PATH})

