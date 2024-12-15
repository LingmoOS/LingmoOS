cmake_minimum_required(VERSION 3.7)

set(PLUGIN_NAME "oceanui-network-plugin")

project(${PLUGIN_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "-g -Wall")

# 增加安全编译参数
ADD_DEFINITIONS("-fstack-protector-strong -D_FORTITY_SOURCE=1 -z noexecstack -pie -fPIC -z lazy")

# 指定所有源码文件
# file(GLOB_RECURSE SRCS "*.h"
#     "*.cpp"
# )

# 指定要用到的库
# -DQT_MAJOR_VERSION=6
# find_package(QT NAMES Qt6 Qt5)
# set(QT_MAJOR_VERSION 6)
# set(QT_DEFAULT_MAJOR_VERSION 6)
# find_package(Qt6 REQUIRED COMPONENTS Quick)
find_package(Qt6 COMPONENTS Core Gui DBus Network LinguistTools REQUIRED)
find_package(DdeControlCenter REQUIRED)
find_package(Dtk6 COMPONENTS Core REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(KF6NetworkManagerQt REQUIRED)
#pkg_check_modules(OCEAN-Network-Core REQUIRED ocean-network-core)


set(OceanUINetwork_Name network)
file(GLOB_RECURSE OceanUINetwork_SRCS
        "*.cpp"
        "*.h"
        "../net-view/operation/*.cpp"
        "../net-view/operation/*.h"
        "../net-view/window/qrc/network.qrc"
        # "qrc/systeminfo.qrc"
)

add_library(${OceanUINetwork_Name} MODULE
        ${OceanUINetwork_SRCS}
)

target_include_directories(${OceanUINetwork_Name} PUBLIC
    Dde::Control-Center
    "../net-view/operation"
    "../src"
)

set(OceanUINetwork_Libraries
        Qt6::DBus
        Qt6::Network
        Qt6::Gui
        # Qt6::Quick
        Dtk6::Core
        KF6::NetworkManagerQt
        Dde::Control-Center
        ocean-network-core6
)

target_link_libraries(${OceanUINetwork_Name} PRIVATE
        ${OceanUINetwork_Libraries}
)

oceanui_install_plugin(NAME ${OceanUINetwork_Name} TARGET ${OceanUINetwork_Name})

oceanui_handle_plugin_translation(NAME ${OceanUINetwork_Name} )
