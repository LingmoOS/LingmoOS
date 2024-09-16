## 定义需要的cmake版本
#cmake_minimum_required(VERSION 3.10)

#SET(DEEPIN_DRAW_LIB "deepinDrawBase" CACHE STRING INTERNAL FORCE)

## 设置cmake参数
#set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_INCLUDE_CURRENT_DIR ON)
#set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTORCC ON)
#set(CMAKE_AUTOUIC ON)

##set complier/link flags
#if(DEEPINDRAW_TEST)
#    message(DEEPINDRAW_TESTDEEPINDRAW_TESTDEEPINDRAW_TESTDEEPINDRAW_TEST)
#    #0.DEEPINDRAW_TEST mean that we should creat the lib that can get cover info
#    #mips not suport -fsanitize
#    if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "mips64")
#        set(CMAKE_CXX_FLAGS "-g -O1 -fprofile-arcs -ftest-coverage")
#        set(CMAKE_C_FLAGS "-g -O1 -fprofile-arcs -ftest-coverage")
#        set(CMAKE_Ｌ_FLAGS "-g -O1 -fprofile-arcs -ftest-coverage -lgcov")
#    else()
#        message(DEEPINDRAW_TESTDEEPINDRAW_TESTDEEPINDRAW_TESTDEEPINDRAW_TEST1)
#        set(CMAKE_CXX_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer -g -O1 -fprofile-arcs -ftest-coverage")
#        set(CMAKE_C_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer -g -O1 -fprofile-arcs -ftest-coverage")
#        set(CMAKE_Ｌ_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer -g -O1 -fprofile-arcs -ftest-coverage -lgcov")
#    endif()
#else()
#    #1.no DEEPINDRAW_TEST mean that we just need one clearly lib(not include -ftest-coverage or -fsanitize)
#    if(CMAKE_BUILD_TYPE MATCHES "Debug")
#        #DEBUG时不做优化方便调试
#        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
#        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
#    else()
#        #加速编译优化参数
#        if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "mips64")
#            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wl,-O1 -Wl,--as-needed -Wl,-E -fPIE -ffunction-sections -fdata-sections -Wl,--gc-sections -ftree-vectorize -march=loongson3a -mhard-float -mno-micromips -mno-mips16 -flax-vector-conversions -mloongson-ext2 -mloongson-mmi")
#            SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -Wl,-O1 -Wl,--as-needed -Wl,-E -fPIE -ffunction-sections -fdata-sections -Wl,--gc-sections -ftree-vectorize -march=loongson3a -mhard-float -mno-micromips -mno-mips16 -flax-vector-conversions -mloongson-ext2 -mloongson-mmi")
#            SET(CMAKE_EXE_LINKER_FLAGS  "-pie")
#        else()
#            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wl,-O1 -Wl,--as-needed -Wl,-E -fPIE -ffunction-sections -fdata-sections -Wl,--gc-sections")
#            SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3  -Wl,-O1 -Wl,--as-needed -Wl,-E -fPIE -ffunction-sections -fdata-sections -Wl,--gc-sections")
#            SET(CMAKE_EXE_LINKER_FLAGS  "-pie")
#        endif()
#    endif()

#    #2.sometime we also want to test deepin-draw memery info
#    if(DEEPINDRAW_SANITIZERS)
#        set(CMAKE_CXX_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer")
#        set(CMAKE_C_FLAGS "-fsanitize=undefined,address,leak -fno-omit-frame-pointer")
#    endif()
#endif()

## 设置Qt模块
#set(QtModule Core Gui Widgets DBus PrintSupport Svg Concurrent LinguistTools)

## 查找Qt相关的库链接
#find_package(Qt5 REQUIRED ${QtModule})

#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../config.h.in ${PROJECT_BINARY_DIR}/config.h @ONLY)

#set(SRCDIRS
#    ${CMAKE_CURRENT_SOURCE_DIR}
#    ${CMAKE_CURRENT_SOURCE_DIR}/drawshape
#    ${CMAKE_CURRENT_SOURCE_DIR}/drawshape/drawTools
#    ${CMAKE_CURRENT_SOURCE_DIR}/drawshape/drawItems
#    ${CMAKE_CURRENT_SOURCE_DIR}/drawshape/drawItems/bzItems
#    ${CMAKE_CURRENT_SOURCE_DIR}/frame
#    ${CMAKE_CURRENT_SOURCE_DIR}/service
#    ${CMAKE_CURRENT_SOURCE_DIR}/utils
#    ${CMAKE_CURRENT_SOURCE_DIR}/widgets
#    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/dialog
#    ${CMAKE_CURRENT_SOURCE_DIR}/frame/AttributesWidgets
#    ${CMAKE_CURRENT_SOURCE_DIR}/frame/AttributesWidgets/private
#    ${CMAKE_CURRENT_SOURCE_DIR}/res
#    )
#SET(BASE_LIB_INCLUDEDIRS "${SRCDIRS}" CACHE STRING INTERNAL FORCE)

## 包含源码文件夹并获取其下所有源代码文件
#foreach(item IN LISTS SRCDIRS)
#    include_directories(${item})
#    list(APPEND SRC_CPPS    ${item}/*.cpp)
#    list(APPEND SRC_HEADERS ${item}/*.h)
#    list(APPEND SRC_QRCS    ${item}/*.qrc)
#endforeach()
#FILE (GLOB allSource  ${SRC_CPPS})
#FILE (GLOB allHeaders ${SRC_HEADERS})
#FILE (GLOB allQrcs    ${SRC_QRCS})

#FILE(GLOB targetTsFiles ../translations/*.ts)

## 创建翻译文件(将源文件${allSource} ${allHeaders}关联上${targetTsFiles}的ts文件 然后生成${tragetQmFiles}的qm文件)
#qt5_create_translation(tragetQmFiles ${allSource} ${allHeaders} ${targetTsFiles})
##qt5_create_translation(tragetQmFiles ${allSource} ${allHeaders} ${targetTsFiles} OPTIONS -no-obsolete)

## 设置不删除生成的文件夹内容文件（设置此项是为了翻译后的文件不被删除）
#set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1)

#qt5_add_resources(RESOURCE_ADDED ${allQrcs})
#if(DEEPINDRAW_LINK_DRAWBASELIB_STATIC)
#    add_library(${DEEPIN_DRAW_LIB} STATIC  ${allSource} ${allHeaders} ${RESOURCE_ADDED} ${tragetQmFiles})
#else()
#    add_library(${DEEPIN_DRAW_LIB} SHARED  ${allSource} ${allHeaders} ${RESOURCE_ADDED} ${tragetQmFiles})
#endif()

#set_target_properties(${DEEPIN_DRAW_LIB}
#  PROPERTIES
#  VERSION 0.0
#  SOVERSION 0
#  PUBLIC_HEADER "${allHeaders}")

## Find includes in corresponding build directories
#set(CMAKE_INCLUDE_CURRENT_DIR ON)
#include_directories(${PROJECT_BINARY_DIR})
#include_directories(${PROJECT_SOURCE_DIR})

##------------------------------ 添加第三方库  DTK -------------------------------
## 使用第三方库需要用到的一个包
#find_package(PkgConfig REQUIRED)

## 检查第三方库(这里检查了名字为dtkwidget的库和名字为dtkgui的库)，然后取名3RMODULES
#pkg_check_modules(3RMODULES REQUIRED
#        dtkwidget dtkgui
#        )
## 添加第三方库的所有文件夹路径到工程中来(注意 *_INCLUDE_DIRS)
#target_include_directories(${DEEPIN_DRAW_LIB} PUBLIC ${3RMODULES_INCLUDE_DIRS})
#target_link_libraries(${DEEPIN_DRAW_LIB} ${3RMODULES_LIBRARIES} ${CMAKE_DL_LIBS})

##------------------------------添加第三方库end-------------------------------------

## 将工程与Qt模块链接起来
#qt5_use_modules(${DEEPIN_DRAW_LIB} ${QtModule})

## ------------------------------- install script -------------------------------
#set(DbusDir ${PREFIX}/share/dbus-1/services/)
#set(DesktopDir ${PREFIX}/share/applications/)
#set(ManIconDir ${PREFIX}/share/icons/deepin/apps/scalable)
#set(AppIconDir ${PREFIX}/share/icons/hicolor/scalable/apps)
#set(MimeFileDir ${PREFIX}/share/mime/packages)
#set(MimeTypeDir ${PREFIX}/share/application)
#set(HelpFileDir ${PREFIX}/share/deepin-manual/manual-assets/application)

#if(NOT DEEPINDRAW_LINK_DRAWBASELIB_STATIC)
#    install(TARGETS ${DEEPIN_DRAW_LIB}
#        LIBRARY DESTINATION /usr/lib
#        ARCHIVE DESTINATION /usr/lib
#        PUBLIC_HEADER DESTINATION /usr/include/${PROJECT_NAME})
#endif()

## install Translation files:
#install(FILES ${tragetQmFiles} DESTINATION ${TranslationDir})

## install Manual icon
#install(FILES ./logo/deepin-draw.svg DESTINATION ${ManIconDir})

## install App icon
#install(FILES ./logo/deepin-draw.svg DESTINATION ${AppIconDir})

## install Mimetype
#install(FILES ./service/x-ddf.xml DESTINATION ${MimeTypeDir})

add_subdirectory(drawboard)
add_subdirectory(deepin-draw)


