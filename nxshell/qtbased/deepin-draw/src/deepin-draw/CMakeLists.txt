# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

# 设置cmake参数
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 设置工程名字
project(deepin-draw)

#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../../config.h.in ${PROJECT_BINARY_DIR}/config.h @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../../config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/drawConfig.h @ONLY)

# 设置Qt模块
set(QtModule Core Gui Widgets DBus PrintSupport Svg Concurrent LinguistTools)

# 查找Qt相关的库链接
find_package(Qt5 REQUIRED ${QtModule})

set(SRCDIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets
    ${CMAKE_CURRENT_SOURCE_DIR}/drawtools
    ${CMAKE_CURRENT_SOURCE_DIR}/drawitems
    ${CMAKE_CURRENT_SOURCE_DIR}/drawfiles
    ${CMAKE_CURRENT_SOURCE_DIR}/attribution
    ${CMAKE_CURRENT_SOURCE_DIR}/attribution/stylewidgets
    ${CMAKE_CURRENT_SOURCE_DIR}/attribution/attrbuionregisters
    ${CMAKE_CURRENT_SOURCE_DIR}/../res/attribution
    ${CMAKE_CURRENT_SOURCE_DIR}/../res/interface
    ${CMAKE_CURRENT_SOURCE_DIR}/../res/theme
    ${DRAW_BOARD_LIB_RES}
    )
#包含所有路径用于UT
SET(BASE_DRAW_INCLUDEDIRS "${SRCDIRS}" CACHE STRING INTERNAL FORCE)

# 包含源码文件夹并获取其下所有源代码文件
foreach(item IN LISTS SRCDIRS)
    include_directories(${item})
    list(APPEND SRC_CPPS1    ${item}/*.cpp)
    list(APPEND SRC_HEADERS1 ${item}/*.h)
    list(APPEND SRC_QRCS1    ${item}/*.qrc)
endforeach()
FILE (GLOB allSource1  ${SRC_CPPS1})
FILE (GLOB allHeaders1 ${SRC_HEADERS1})
FILE (GLOB allQrcs1    ${SRC_QRCS1})

include_directories(${BASE_LIB_INCLUDEDIRS})

qt5_add_resources(RESOURCE_ADDED ${allQrcs1})
add_executable(${PROJECT_NAME}  ${allSource1} ${allHeaders1} ${RESOURCE_ADDED} )

target_include_directories(${PROJECT_NAME} PUBLIC ${3RMODULES_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} ${3RMODULES_LIBRARIES} ${CMAKE_DL_LIBS} ${BASE_LIB})


# 将工程与Qt模块链接起来
qt5_use_modules(${PROJECT_NAME} ${QtModule})

# ------------------------------- install script -------------------------------
set(DbusDir ${PREFIX}/share/dbus-1/services/)
set(DesktopDir ${PREFIX}/share/applications/)
set(ManIconDir ${PREFIX}/share/icons/deepin/apps/scalable)
set(AppIconDir ${PREFIX}/share/icons/hicolor/scalable/apps)
set(MimeFileDir ${PREFIX}/share/mime/packages)
set(MimeTypeDir ${PREFIX}/share/application)
set(HelpFileDir ${PREFIX}/share/deepin-manual/manual-assets/application)

install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Targets
        LIBRARY DESTINATION ${PREFIX}/lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION ${PREFIX}/bin
        INCLUDES DESTINATION allHeaders)

# install DBus file:
install(FILES ../../com.deepin.Draw.service DESTINATION ${DbusDir})

# install Desktop icon
install(FILES ../../deepin-draw.desktop DESTINATION ${DesktopDir})

# install MimeFile
install(FILES ../../mimetype/deepin-draw.xml DESTINATION ${MimeFileDir})

# install Help File
install(DIRECTORY ../../assets/deepin-draw DESTINATION ${HelpFileDir})

