# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIE -pie")

# 设置库名
set(LIB_NAME ${DAEMON_LIB_NAME})

# 依赖包
find_package(PkgConfig REQUIRED)
find_package(DtkCore REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(Qt5 COMPONENTS
    Core
    Gui
    Widgets
    Concurrent
    DBus
REQUIRED)

set(Qt_LIBS
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::DBus
    Qt5::Concurrent
)

# 依赖库
set(LINK_LIBS
    ${Qt_LIBS}
    ${DtkCore_LIBRARIES}
    ${DtkWidget_LIBRARIES}
)

# 源文件
set(DBUS
    dbusservice/grandsearchinterface_p.h
    dbusservice/grandsearchinterface.h
    dbusservice/grandsearchinterface.cpp
)

set(MAINCTRL
    maincontroller/maincontroller.h
    maincontroller/maincontroller_p.h
    maincontroller/maincontroller.cpp
    maincontroller/task/taskcommander.h
    maincontroller/task/taskcommander.cpp
    maincontroller/task/taskcommander_p.h
)

FILE(GLOB_RECURSE SEMANTIC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/searcher/semantic/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/searcher/semantic/*.cpp"
)

set(SEARCHER
    searcher/searchergroup.h
    searcher/searchergroup_p.h
    searcher/searchergroup.cpp
    searcher/searcher.h
    searcher/searcher.cpp
    searcher/proxyworker.h
    searcher/proxyworker.cpp
    searcher/app/desktopappsearcher.h
    searcher/app/desktopappsearcher_p.h
    searcher/app/desktopappsearcher.cpp
    searcher/app/desktopappworker.h
    searcher/app/desktopappworker.cpp
    searcher/web/statictextechoer.h
    searcher/web/statictextechoer.cpp
    searcher/web/statictextworker.h
    searcher/web/statictextworker.cpp
    searcher/extend/extendsearcher.h
    searcher/extend/extendsearcher_p.h
    searcher/extend/extendsearcher.cpp
    searcher/extend/extendworker.h
    searcher/extend/extendworker.cpp
)

#文件搜索
if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "mips"
        OR ${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64"
        OR ${CMAKE_SYSTEM_PROCESSOR} MATCHES "sw"
        OR ${CMAKE_SYSTEM_PROCESSOR} MATCHES "loongarch64"
        )
    message(${CMAKE_SYSTEM_PROCESSOR} ": using fsearch")
    ADD_DEFINITIONS(-DENABLE_FSEARCH)
    FILE(GLOB 3RD_FSEARCH_SRC ${PROJECT_3RDPARTY_PATH}/fsearch/*)
    pkg_check_modules(GLIB REQUIRED glib-2.0)
    pkg_check_modules(PCRE REQUIRED libpcre)
    set(SEARCHER
        ${SEARCHER}
        ${3RD_FSEARCH_SRC}
        searcher/file/fssearcher.h
        searcher/file/fssearcher.cpp
        searcher/file/fsworker.h
        searcher/file/fsworker.cpp
        searcher/file/filesearchutils.h
        searcher/file/filesearchutils.cpp
    )
    set(LINK_LIBS
        ${LINK_LIBS}
        ${GLIB_LIBRARIES}
        ${PCRE_LIBRARIES}
    )
else ()
    message(${CMAKE_SYSTEM_PROCESSOR} ": using deepin-anything.")
    ADD_DEFINITIONS(-DENABLE_DEEPINANYTHING)
    set(SEARCHER
        ${SEARCHER}
        searcher/file/filenamesearcher.h
        searcher/file/filenamesearcher.cpp
        searcher/file/filenameworker.h
        searcher/file/filenameworker.cpp
        searcher/file/filenameworker_p.h
        searcher/file/filesearchutils.h
        searcher/file/filesearchutils.cpp
        ${PROJECT_3RDPARTY_PATH}/interfaces/anything_interface.h
        ${PROJECT_3RDPARTY_PATH}/interfaces/anything_interface.cpp
        ${SEMANTIC_FILES}
    )

    set(LINK_LIBS
        ${LINK_LIBS}
        semanticparser
        luceneengine
    )
endif()

# 手动生成xml描述文件
# qdbuscpp2xml -A grandsearchplugininterfacev1.h -o com.deepin.dde.grandsearch.searchplugin.xml
set(PLUGIN_ABSTRACT_IFS
    searchplugin/interface/abstract/grandsearchplugininterfacev1.h
)

# 插件通信接口
set(SEARCHPLUGIN_IFS
    searchplugin/interface/searchplugininterfacev1.h
    searchplugin/interface/searchplugininterfacev1.cpp
)

# 插件通信协议
set(SEARCHPLUGIN_PROT
    searchplugin/convertors/abstractconvertor.h
    searchplugin/convertors/abstractconvertor.cpp
    searchplugin/convertors/convertorv1_0.h
    searchplugin/convertors/convertorv1_0.cpp
)

set(SEARCHPLUGIN
    searchplugin/pluginmanager.h
    searchplugin/pluginmanager_p.h
    searchplugin/pluginmanager.cpp
    searchplugin/loader/pluginloader.h
    searchplugin/loader/pluginloader.cpp
    searchplugin/process/pluginprocess.h
    searchplugin/process/pluginprocess.cpp
    searchplugin/convertors/dataconvertor.h
    searchplugin/convertors/dataconvertor.cpp
    searchplugin/pluginliaison.h
    searchplugin/pluginliaison_p.h
    searchplugin/pluginliaison.cpp
    ${SEARCHPLUGIN_IFS}
    ${SEARCHPLUGIN_PROT}
)

set(GDCONFIGURATION
    configuration/configer.h
    configuration/configer_p.h
    configuration/configer.cpp
    configuration/preferenceitem.h
    configuration/userpreference.h
    configuration/userpreference.cpp
)

set(UTILS
    utils/searchpluginconf.h
    utils/searchplugininfo.h
    utils/searchpluginprotocol.h
    utils/specialtools.h
    utils/specialtools.cpp
    utils/chineseletterhelper.h
    utils/chineseletterhelper.cpp
)

set(SRCS
    main.cpp
    ${GLOBAL_SRC}
    ${UTILS}
    ${DBUS}
    ${MAINCTRL}
    ${SEARCHER}
    ${SEARCHPLUGIN}
    ${GDCONFIGURATION}
)

# 资源文件
qt5_add_resources(QRC_FILES utils/dict.qrc)

# 库
add_library(${LIB_NAME}
    SHARED
    ${SRCS} ${QRC_FILES}
    )

# 默认会生成liblib开头的库，这里重命名生成lib开头
set_target_properties(${LIB_NAME} PROPERTIES OUTPUT_NAME ${DAEMON_BIN_NAME})

target_include_directories(${LIB_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${DtkWidget_INCLUDE_DIRS}
    ${PROJECT_3RDPARTY_PATH}/fsearch
    ${GLIB_INCLUDE_DIRS}
    ${PCRE_INCLUDE_DIRS}
)

target_link_libraries(${LIB_NAME} PRIVATE
    ${LINK_LIBS}
)

# lib
install(TARGETS ${LIB_NAME} LIBRARY DESTINATION ${DAEMON_LIB_BASE_DIR})

# 扩展搜索插件目录
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/.readme DESTINATION ${PLUGIN_SEARCHER_DIR})
