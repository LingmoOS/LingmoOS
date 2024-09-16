# 学习 cmake
# 时建议直接从命令列表作为入口，遇到不清楚意思的命令都可以在此处查阅：
# https://cmake.org/cmake/help/latest/manual/cmake-commands.7.html
# 另外下面是完整的文档入口：
# https://cmake.org/cmake/help/latest/

# 设置运行该脚本所需的 cmake 最低版本
cmake_minimum_required(VERSION 3.9)

# 使用 set 命令设置一个变量
set(PROJECT_NAME deepin-system-upgrade-tool)
# set(TEST_NAME uos-updater-test)

# 设置项目名称
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
# 启用 qt moc 的支持
set(CMAKE_AUTOMOC ON)
# 启用 qrc 资源文件的支持
set(CMAKE_AUTORCC ON)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O1 -DNDEBUG")

# 指定所有源码文件
# 使用了 cmake 的 file 命令，递归查找项目目录下所有头文件和源码文件，
# 并将结果放入 SRCS 变量中，SRCS 变量可用于后续使用

file(GLOB CORE_SRCS
    "src/core/*.h"
    "src/core/*.cpp"
)

file(GLOB SETTINGS_SRCS
    "src/settings/*.h"
    "src/settings/*.cpp"
)

file(GLOB WIDGETS_SRCS
    "src/widgets/*.h"
    "src/widgets/*.cpp"
)

file(GLOB TOOL_SRCS
    "src/tool/*.h"
    "src/tool/*.cpp"
    "src/tool/image_preparation/*.cpp"
    "src/tool/upgrade_check/*.cpp"
    "src/tool/upgrade_progress/*.cpp"
    "src/tool/migrate_apps/*.cpp"
)

file(GLOB TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/misc/translations/*.ts)

# 指定要用到的库
# 使用了 cmake 的 find_package 命令，查找库 Qt5Widgets 等，
# REQUIRED 参数表示如果没有找到则报错
# find_package 命令在找到并加载指定的库之后会设置一些变量，
# 常用的有：
# <库名>_FOUND          是否找到（Qt5Widgets_FOUND）
# <库名>_DIR            在哪个目录下找到的（Qt5Widgets_DIR）
# <库名>_INCLUDE_DIRS   有哪些头文件目录（Qt5Widgets_INCLUDE_DIRS）
# <库名>_LIBRARIES      有哪些库文件（Qt5Widgets_LIBRARIES）

find_package(DtkWidget REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets Concurrent X11Extras DBus Network)

# find_package 命令还可以用来加载 cmake 的功能模块
# 并不是所有的库都直接支持 cmake 查找的，但大部分都支持了 pkg-config 这个标准，
# 因此 cmake 提供了间接加载库的模块：FindPkgConfig， 下面这行命令表示加载 FindPkgConfig 模块，
# 这个 cmake 模块提供了额外的基于 “pkg-config” 加载库的能力
# 执行下面的命令后后会设置如下变量，不过一般用不到：
# PKG_CONFIG_FOUND            pkg-config 可执行文件是否找到了
# PKG_CONFIG_EXECUTABLE       pkg-config 可执行文件的路径
# PKG_CONFIG_VERSION_STRING   pkg-config 的版本信息

set(INCLUDE_DIRECTORIES
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS}
    ${DtkWidget_INCLUDE_DIRS}
    ${Qt5DBus_INCLUDE_DIRS}
    ${Qt5Concurrent_INCLUDE_DIRS}
)

set(LINK_LIBRARIES
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Network_LIBRARIES}
    ${DtkWidget_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${Qt5Concurrent_LIBRARIES}
)

add_compile_definitions(CVERSION="${CVERSION}")

add_executable(${PROJECT_NAME}
    ${CORE_SRCS}
    ${SETTINGS_SRCS}
    ${WIDGETS_SRCS}
    ${TOOL_SRCS}
    ${QM_FILES}
    misc/resource.qrc
)

# Generate QM files
# For qt_*_translation don't work.
execute_process(COMMAND lupdate ${CMAKE_CURRENT_SOURCE_DIR}/src -ts ${TS_FILES}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/misc/translations)
execute_process(COMMAND lrelease ${TS_FILES}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/misc/translations)
file(GLOB QM_FILES "${CMAKE_CURRENT_SOURCE_DIR}/misc/translations/*.qm")

target_include_directories(${PROJECT_NAME} PUBLIC
    ${INCLUDE_DIRECTORIES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${LINK_LIBRARIES}
)

# 设置安装路径的前缀(默认为"/usr/local")
set(CMAKE_INSTALL_PREFIX "/usr")

# icon
install(FILES misc/icons/upgrade-tool.svg DESTINATION share/icons/hicolor/scalable/apps)

# desktop file
install(FILES misc/deepin-system-upgrade-tool.desktop  DESTINATION share/applications)

# plymouth files
install(DIRECTORY misc/plymouth/themes/deepin-upgrade DESTINATION share/plymouth/themes/)

# translation files
install(FILES ${QM_FILES} DESTINATION share/${PROJECT_NAME}/translations)

# 设置执行 make install 时哪个目标应该被 install 到哪个位置
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
