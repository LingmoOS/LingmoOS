# 定义需要的cmake版本
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

# 定义插件名称
set(PLUGIN_NAME ddegrandsearch_dockplugin)

# 设置项目名称
project(${PLUGIN_NAME})

# 指定源码文件
set(SRCS
    ddegrandsearchdockplugin.h
    ddegrandsearchdockplugin.cpp
    versiondefine.h
    gui/grandsearchwidget.h
    gui/grandsearchwidget.cpp
    gui/tipswidget.h
    gui/tipswidget.cpp
    ${PROJECT_3RDPARTY_PATH}/interfaces/grandsearchinterface.h
    ${PROJECT_3RDPARTY_PATH}/interfaces/grandsearchinterface.cpp
    )

# 指定资源文件
set(QRCS
    gui/resource.qrc
    )

# 指定要用到的库
find_package(Qt5Widgets REQUIRED)
find_package(DtkWidget REQUIRED)


# 加载 FindPkgConfig 模块
find_package(PkgConfig REQUIRED)

# 加载需要的库
pkg_check_modules(DdeDockInterface REQUIRED dde-dock)
pkg_check_modules(QGSettings REQUIRED gsettings-qt)

# 编译目标
add_library(${PLUGIN_NAME} SHARED ${SRCS} ${QRCS})


# 目标位置
set_target_properties(${PLUGIN_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ./)

# 设置目标要使用的 include 目录，即头文件目录
# 变量 ${DtkWidget_INCLUDE_DIRS} 是在前面执行 find_package 命令时引入的
# 当出现编译失败提示找不到某些库的头文件时应该检查此处是否将所有需要的头文件都包含了
target_include_directories(${PLUGIN_NAME} PUBLIC
    ${Qt5Widgets_INCLUDE_DIRS}
    ${DtkWidget_INCLUDE_DIRS}
    ${DdeDockInterface_INCLUDE_DIRS}
    ${QGSettings_INCLUDE_DIRS}
)

# 设置目标要使用的链接库
# 变量 ${DtkWidget_LIBRARIES} 和 ${Qt5Widgets_LIBRARIES} 是在前面执行 find_package 命令时引入的
# 当出现运行时错误提示某些符号没有定义时应该检查此处是否将所有用的库都写在了这里
target_link_libraries(${PLUGIN_NAME} PRIVATE
    ${Qt5Widgets_LIBRARIES}
    ${DtkWidget_LIBRARIES}
    ${DdeDockInterface_LIBRARIES}
    ${QGSettings_LIBRARIES}
)

# 设置安装路径的前缀(默认为"/usr/local")
set(CMAKE_INSTALL_PREFIX "/usr")

# 设置执行 make install 时哪个目标应该被 install 到哪个位置
# 插件.so在/usr/lib/dde-dock/plugins/下则显示在插件区域,在/usr/lib/dde-dock/plugins/system-tray/目录则显示在托盘区域
install(TARGETS ${PLUGIN_NAME} LIBRARY DESTINATION lib/dde-dock/plugins/)

# gsettings
install(FILES gschema/com.deepin.dde.dock.module.grand-search.gschema.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/glib-2.0/schemas)


