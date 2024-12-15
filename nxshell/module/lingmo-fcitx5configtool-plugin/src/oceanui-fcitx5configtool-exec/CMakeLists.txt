cmake_minimum_required(VERSION 3.7)

set(BIN_NAME "oceanui-fcitx5configtool-exec")

project(${BIN_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "-g -Wall")

# 增加安全编译参数
ADD_DEFINITIONS("-fstack-protector-strong -D_FORTITY_SOURCE=1 -z noexecstack -pie -fPIC -z lazy")

# 指定要用到的库
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
find_package(Fcitx5Qt6DBusAddons REQUIRED)

file(GLOB_RECURSE OceanUIFcitx5configtool_SRCS
    "*.cpp"
    "*.h"
)

add_executable(${BIN_NAME}
    ${OceanUIFcitx5configtool_SRCS}
)

set(OceanUIFcitx5configtool_Libraries
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_MAJOR_VERSION}::Widgets
        Dtk${QT_MAJOR_VERSION}::Core
)

target_link_libraries(${BIN_NAME} PRIVATE
        ${OceanUIFcitx5configtool_Libraries}
        configlib${QT_MAJOR_VERSION}
        configwidgetslib${QT_MAJOR_VERSION}
        Fcitx5Qt6::DBusAddons
)

target_include_directories(${BIN_NAME}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src/lib/configwidgetslib
)

message(STATUS "Install ${BIN_NAME} to ${CONFIGTOOL_EXEC_DIR}")
message(STATUS "CONFIGTOOL_EXEC_DIR=${CONFIGTOOL_EXEC_DIR}")
install(TARGETS ${BIN_NAME} DESTINATION ${CONFIGTOOL_EXEC_DIR})
