project (lingdecoration)
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(KF5CoreAddons 5...6 REQUIRED)
find_package(KF5Config 5...6 REQUIRED)
find_package(KF5WindowSystem 5...6 REQUIRED)
find_package(KDecoration2 5...6 REQUIRED)

message(STATUS "KDecoration2 version: ${KDecoration2_VERSION}")
string(REPLACE "." ";" version_parts ${KDecoration2_VERSION})
list(GET version_parts 0 KDECORATION_MAJOR)
list(GET version_parts 1 KDECORATION_MINOR)
list(GET version_parts 2 KDECORATION_PATCH)
add_compile_definitions(KDECORATION_MAJOR=${KDECORATION_MAJOR})
add_compile_definitions(KDECORATION_MINOR=${KDECORATION_MINOR})
add_compile_definitions(KDECORATION_PATCH=${KDECORATION_PATCH})

find_package(Qt5 CONFIG REQUIRED COMPONENTS Gui Widgets Core X11Extras)

find_package(Qt5 CONFIG REQUIRED COMPONENTS Core Gui)

# 获取qmake
get_target_property(QT_QMAKE_EXECUTABLE ${Qt5Core_QMAKE_EXECUTABLE} IMPORTED_LOCATION)
if(NOT QT_QMAKE_EXECUTABLE)
    message(FATAL_ERROR "qmake is not found.")
endif()

# exeling the command "qmake -query QT_INSTALL_PLUGINS" to get the path of plugins dir.
execute_process(COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_PLUGINS
        OUTPUT_VARIABLE QT_PLUGINS_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(QT_PLUGINS_DIR)
    message(STATUS "Qt5 plugin directory:" "${QT_PLUGINS_DIR}")
else()
    message(FATAL_ERROR "Qt5 plugin directory cannot be detected.")
endif()

set (decoration_SRCS
    decoration.cpp
    x11shadow.cpp
    button.cpp
    resources.qrc
)

add_library (lingdecoration MODULE
    ${decoration_SRCS}
)

target_link_libraries (lingdecoration
    PUBLIC
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::X11Extras
        KF5::ConfigCore
        KF5::ConfigGui
        KF5::CoreAddons
        KF5::WindowSystem

    PRIVATE
        KDecoration2::KDecoration
)

install (TARGETS lingdecoration
         DESTINATION ${QT_PLUGINS_DIR}/org.kde.kdecoration2)
