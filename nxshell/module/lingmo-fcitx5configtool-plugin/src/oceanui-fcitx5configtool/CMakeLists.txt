cmake_minimum_required(VERSION 3.7)

set(PLUGIN_NAME "oceanui-fcitx5configtool")

project(${PLUGIN_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "-g -Wall")

# 增加安全编译参数
ADD_DEFINITIONS("-fstack-protector-strong -D_FORTITY_SOURCE=1 -z noexecstack -pie -fPIC -z lazy")

# 指定要用到的库
# -DQT_MAJOR_VERSION=6
# find_package(QT NAMES Qt6 Qt5)
# set(QT_MAJOR_VERSION 6)
# set(QT_DEFAULT_MAJOR_VERSION 6)
find_package(Qt6 COMPONENTS Core Gui DBus LinguistTools Qml REQUIRED)
find_package(DdeControlCenter REQUIRED)
find_package(Dtk6 COMPONENTS Core REQUIRED)
find_package(PkgConfig REQUIRED)


set(OceanUIFcitx5configtool_Name fcitx5configtool)
file(GLOB_RECURSE OceanUIFcitx5configtool_SRCS
        "operation/qrc/*.qrc"
        "*.cpp"
        "*.h"
        "private/*.h"
        "operation/*.cpp"
        "operation/*.h"
)

add_library(${OceanUIFcitx5configtool_Name} MODULE
        ${OceanUIFcitx5configtool_SRCS}
)

target_include_directories(${OceanUIFcitx5configtool_Name} PUBLIC
    Dde::Control-Center
    configlib${QT_MAJOR_VERSION}
    configwidgetslib${QT_MAJOR_VERSION}
)

set(OceanUIFcitx5configtool_Libraries
        Qt${QT_MAJOR_VERSION}::DBus
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_MAJOR_VERSION}::Qml
        Dtk${QT_MAJOR_VERSION}::Core
        Dde::Control-Center
)

target_link_libraries(${OceanUIFcitx5configtool_Name} PRIVATE
        ${OceanUIFcitx5configtool_Libraries}
        configlib${QT_MAJOR_VERSION}
        configwidgetslib${QT_MAJOR_VERSION}
)

oceanui_install_plugin(NAME ${OceanUIFcitx5configtool_Name} TARGET ${OceanUIFcitx5configtool_Name})
oceanui_handle_plugin_translation(NAME ${OceanUIFcitx5configtool_Name} )
