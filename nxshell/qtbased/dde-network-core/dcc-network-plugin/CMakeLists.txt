cmake_minimum_required(VERSION 3.7)

set(PLUGIN_NAME "dcc-network-plugin")

project(${PLUGIN_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "-g -Wall")

# 增加安全编译参数
ADD_DEFINITIONS("-fstack-protector-strong -D_FORTITY_SOURCE=1 -z noexecstack -pie -fPIC -z lazy")

# 指定所有源码文件
file(GLOB_RECURSE SRCS "*.h"
    "*.cpp"
    "../src/*.h"
    "../src/*.cpp"
    "*.qrc"
    "*.svg"
)

# 指定要用到的库
find_package(Qt5 COMPONENTS Core Widgets DBus Network LinguistTools REQUIRED)
find_package(DdeControlCenter REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(KF5NetworkManagerQt REQUIRED)
#pkg_check_modules(DDE-Network-Core REQUIRED dde-network-core)

get_target_property(KF5_QT_INCLUDE_DIRS KF5::NetworkManagerQt INTERFACE_INCLUDE_DIRECTORIES)

set(TRY_KF5_LIBRARIES
IMPORTED_LOCATION_DEBIAN
IMPORTED_LOCATION_NOCONFIG
)

if (NOT KF5_QT_LIBRARIES)
    find_library(KF5_QT_LIBRARIES libKF5NetworkManagerQt.so)
endif()

file(GLOB TS_FILES "translations/*.ts")

qt5_add_translation(QM_FILES ${TS_FILES})

add_custom_target(translations ALL DEPENDS ${QM_FILES})

# 新增一个编译目标
# 这里使用命令 add_library 来表示本项目要生成一个库文件目标，
add_library(${PLUGIN_NAME} SHARED ${SRCS})

# 设置目标要使用的 include 目录，即头文件目录
# 变量 ${DtkWidget_INCLUDE_DIRS} 是在前面执行 find_package 命令时引入的
# 当出现编译失败提示找不到某些库的头文件时应该检查此处是否将所有需要的头文件都包含了
target_include_directories(${PLUGIN_NAME} PUBLIC
    window
    operation
    ${KF5_QT_INCLUDE_DIRS}
    ${NETINTERFACEINCLUDE}
    ${DDE-Network-Core_INCLUDE_DIRS}
)
# 设置目标要使用的链接库
# 变量 ${DtkWidget_LIBRARIES} 和 ${Qt5Widgets_LIBRARIES} 是在前面执行 find_package 命令时引入的
# 当出现运行时错误提示某些符号没有定义时应该检查此处是否将所有用的库都写在了这里
target_link_libraries(${PLUGIN_NAME} PRIVATE
    ${KF5_QT_LIBRARIES}
    Dtk::Widget
    Qt5::Widgets
    Qt5::Network
    Qt5::DBus
    Dde::DCCWidget
    Dde::DCCInterface
    ${DDE-Network-Core_LIBRARIES}
)

# 设置执行 make install 时哪个目标应该被 install 到哪个位置
install(TARGETS ${PLUGIN_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/dde-control-center/modules)

# 安装 .qm 文件
install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/dde-control-center/translations)
