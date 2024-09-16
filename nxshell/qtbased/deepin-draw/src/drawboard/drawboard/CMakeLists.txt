# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

SET(BASE_LIB "drawboard" CACHE STRING INTERNAL FORCE)

project(${BASE_LIB})

# 设置cmake参数
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_definitions(-DDRAWSHARED_LIBRARY)

# 设置Qt模块
set(QtModule Core Gui Widgets DBus PrintSupport Svg Concurrent LinguistTools)

#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${PROJECT_BINARY_DIR}/config.h @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/utils/config.h @ONLY)

# 查找Qt相关的库链接
find_package(Qt5 REQUIRED ${QtModule})
set(DRAW_BOARD_LIB_RES ${CMAKE_CURRENT_SOURCE_DIR}/res/drawBaseRes CACHE STRING INTERNAL FORCE)

set(SRCDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/attribution
    ${CMAKE_CURRENT_SOURCE_DIR}/dataHander
    ${CMAKE_CURRENT_SOURCE_DIR}/dataHander/ddfHander/ddfProcessor
    ${CMAKE_CURRENT_SOURCE_DIR}/dataHander/ddfHander
    ${CMAKE_CURRENT_SOURCE_DIR}/dataHander/imageHander
    ${CMAKE_CURRENT_SOURCE_DIR}/drawboard
    ${CMAKE_CURRENT_SOURCE_DIR}/drawboard/context
    ${CMAKE_CURRENT_SOURCE_DIR}/drawboard/gui
    ${CMAKE_CURRENT_SOURCE_DIR}/drawItems
    ${CMAKE_CURRENT_SOURCE_DIR}/drawItems/itemData
    ${CMAKE_CURRENT_SOURCE_DIR}/drawItems/itemEvents
    ${CMAKE_CURRENT_SOURCE_DIR}/drawItems/items
    ${CMAKE_CURRENT_SOURCE_DIR}/drawItems/items/commonItems
    ${CMAKE_CURRENT_SOURCE_DIR}/drawtools
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/dialog
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/colorWidget
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/colorWidget/private
    ${CMAKE_CURRENT_SOURCE_DIR}/service
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/res
    ${CMAKE_CURRENT_SOURCE_DIR}/res/drawBaseRes
    ${CMAKE_CURRENT_SOURCE_DIR}/res/toolIcons
    ${CMAKE_CURRENT_SOURCE_DIR}/res/toolIconsNew
    ${CMAKE_CURRENT_SOURCE_DIR}/res/drawBaseResNew
    )
SET(BASE_LIB_INCLUDEDIRS "${SRCDIRS}" CACHE STRING INTERNAL FORCE)

# 包含源码文件夹并获取其下所有源代码文件
foreach(item IN LISTS SRCDIRS)
    include_directories(${item})
    list(APPEND SRC_CPPS    ${item}/*.cpp)
    list(APPEND SRC_HEADERS ${item}/*.h)
    list(APPEND SRC_QRCS    ${item}/*.qrc)
endforeach()
FILE (GLOB allSource  ${SRC_CPPS})
FILE (GLOB allHeaders ${SRC_HEADERS})
FILE (GLOB allQrcs    ${SRC_QRCS})

FILE(GLOB targetTsFiles translations/*.ts)

#翻译是外部人员修改,只需要添加，不需要创建新的ts文件
qt5_add_translation(tragetQmFiles ${targetTsFiles})
#qt5_create_translation(tragetQmFiles ${allSource} ${allHeaders} ${targetTsFiles} OPTIONS -no-obsolete)

# 设置不删除生成的文件夹内容文件（设置此项是为了翻译后的文件不被删除）
set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1)

qt5_add_resources(RESOURCE_ADDED ${allQrcs})

add_library(${BASE_LIB} SHARED  ${allSource} ${allHeaders} ${RESOURCE_ADDED} ${tragetQmFiles})

set_target_properties(${BASE_LIB}
  PROPERTIES
  VERSION 0.0
  SOVERSION 0
  PUBLIC_HEADER "${allHeaders}")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(${PROJECT_BINARY_DIR})
include_directories(${PROJECT_SOURCE_DIR})

#------------------------------ 添加第三方库  DTK -------------------------------
# 使用第三方库需要用到的一个包

find_package(PkgConfig REQUIRED)


message(CMAKE_SYSTEM_NAME = ${CMAKE_SYSTEM_NAME})
# 检查第三方库(这里检查了名字为dtkwidget的库和名字为dtkgui的库)，然后取名3rd_lib
if(CMAKE_SYSTEM_NAME MATCHES "Linux" )
    if(USE_DTK)
        pkg_check_modules(3rd_lib REQUIRED
                dtkwidget dtkgui
                )
    endif()
else()
    pkg_check_modules(3rd_lib REQUIRED
            )
endif()

# 添加第三方库的所有文件夹路径到工程中来(注意 *_INCLUDE_DIRS)
target_include_directories(${BASE_LIB} PUBLIC ${3rd_lib_INCLUDE_DIRS})
target_link_libraries(${BASE_LIB} ${3rd_lib_LIBRARIES} ${CMAKE_DL_LIBS})

#------------------------------添加第三方库end-------------------------------------

# 将工程与Qt模块链接起来
qt5_use_modules(${BASE_LIB} ${QtModule})

# ------------------------------- install script -------------------------------
# install Translation files:
install(FILES ${tragetQmFiles} DESTINATION ${InstallTransDir})
install(TARGETS ${BASE_LIB}
    LIBRARY DESTINATION ${InstallLibDir}
    ARCHIVE DESTINATION ${InstallLibDir}
    PUBLIC_HEADER DESTINATION ${InstallHeaderDir})

#add_subdirectory(toolplugins)

#add_subdirectory(test)

