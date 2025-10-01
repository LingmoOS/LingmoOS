cmake_minimum_required(VERSION 3.16)

project(VolumeOSD LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt5 or Qt6，根据你系统安装的版本调整
find_package(Qt5 COMPONENTS Core Quick QuickControls2 REQUIRED)

# 添加可执行文件
set(RESOURCES
    resources.qrc
)

qt5_add_resources(QT_RESOURCES ${RESOURCES})

add_executable(VolumeOSD
    main.cpp
    VolumeController.cpp
    VolumeController.h
    MouseHelper.h
    ${QT_RESOURCES}
)

# Qt MOC
set_target_properties(VolumeOSD PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)

# 链接 Qt 库
target_link_libraries(VolumeOSD
    Qt5::Core
    Qt5::Quick
    Qt5::QuickControls2
)
