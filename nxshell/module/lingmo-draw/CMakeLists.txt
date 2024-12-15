# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

# 设置cmake参数
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 设置工程名字
project(lingmo-draw)

# 是否开启单元测试编译
option(LINGMODRAW_TEST "only compile and generate lingmo-draw-test program." OFF)

# 是否开启内存泄露检测
option(LINGMODRAW_SANITIZERS "add sanitizers flag for code in gcc.it's a gcc function,but not support on mips." OFF)

# 是否启用accessblity(CI自动化测试.if open, we should add accesibility name for qt widgets.)
option(LINGMODRAW_AC_FUNC_ENABLE "make CI/AT enable.if open, we should add accesibility name for qt widgets." ON)
if(LINGMODRAW_AC_FUNC_ENABLE)
    add_definitions(-DENABLE_ACCESSIBILITY)
endif()

# 是否启用平板模式(平板适配)
option(LINGMODRAW_TABLET_ENABLE "do some outlooking changed with code that could make lingmo-draw run on tablet system." OFF)
if(LINGMODRAW_TABLET_ENABLE)
    add_definitions(-DENABLE_TABLETSYSTEM)
endif()

# if want to support plugins, must generate share drawbaselib instead of static)
option(LINGMODRAW_LINK_DRAWBASELIB_STATIC "static compile lingmo-draw or lingmo-draw-test." ON)
if(LINGMODRAW_LINK_DRAWBASELIB_STATIC)
    add_definitions(-DLINK_DRAWBASELIB_STATIC)
endif()

option(LINGMODRAW_LOAD_TOOL_PLUGINS "if load tool plugins." OFF)
if(LINGMODRAW_LOAD_TOOL_PLUGINS AND NOT LINGMODRAW_LINK_DRAWBASELIB_STATIC)
    add_definitions(-DLOAD_TOOL_PLUGINS)
endif()

#define install destination
set(PREFIX ${CMAKE_INSTALL_PREFIX})
set(BinDir ${PREFIX}/bin)
set(TranslationDir ${PREFIX}/share/lingmo-draw/translations/)
if(LINGMODRAW_LOAD_TOOL_PLUGINS AND NOT LINGMODRAW_LINK_DRAWBASELIB_STATIC)
    set(PluginDir ${PREFIX}/lib/${CMAKE_LIBRARY_ARCHITECTURE}/lingmo-draw/plugins/)
    set(PluginTransDir ${TranslationDir}plugins)
endif()

#1. base lib and main executable
add_subdirectory(src)

#2. test executable
#if(LINGMODRAW_TEST)
#    add_subdirectory(tests)
#endif()

##3.tool plugins(the plugins is for lingmo-draw,so lib of lingmodrawbase(define in ./src) must not be static)
#if(LINGMODRAW_LOAD_TOOL_PLUGINS AND NOT LINGMODRAW_LINK_DRAWBASELIB_STATIC)
#    message(support plugins----------------)
#    add_subdirectory(lingmo-draw-plugins)
#endif()

