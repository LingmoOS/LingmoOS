cmake_minimum_required(VERSION 3.16)

project(system-settings)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_MIN_VERSION "5.12.0")

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Widgets
    Gui
)

find_package(PkgConfig)
pkg_check_modules(Qsettings REQUIRED gsettings-qt)
include_directories(${Qsettings_INCLUDE_DIRS})

file(GLOB_RECURSE sources "*.h" "*.hpp" "*.cpp" "*.c" "*.ui")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${sources})

add_executable(${PROJECT_NAME} ${sources})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Widgets
    Qt5::Core
    Qt5::Gui
    gsettings-qt
)

