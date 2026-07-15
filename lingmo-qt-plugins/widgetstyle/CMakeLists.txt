cmake_minimum_required(VERSION 3.16)
project(lingmostyle)

set(TARGET lingmostyle)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(ECM 6.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(QT Core Gui Widgets DBus)
find_package(Qt6 REQUIRED COMPONENTS ${QT})
find_package(KF6 REQUIRED COMPONENTS WindowSystem)
find_package(X11 REQUIRED)
find_package(X11_XCB REQUIRED)

include(ECMQueryQt)

set (SRCS
    blurhelper.cpp
    blurhelper.h
    pstyleplugin.cpp
    pstyleplugin.h
    basestyle.h
    basestyle.cpp
    phantomcolor.h
    phantomcolor.cpp
    shadowhelper.h
    shadowhelper.cpp
    tileset.h
    tileset.cpp
    boxshadowrenderer.h
    boxshadowrenderer.cpp
)

add_library(${TARGET} MODULE ${SRCS})
target_link_libraries(${TARGET}
    Qt6::GuiPrivate
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
    KF6::WindowSystem
    X11::X11
    X11::XCB
    )

ecm_query_qt(CMAKE_INSTALL_QTPLUGINDIR QT_INSTALL_PLUGINS)

install(TARGETS ${TARGET} DESTINATION ${CMAKE_INSTALL_QTPLUGINDIR}/styles/)
