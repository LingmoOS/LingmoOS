set(TARGET_NAME "deepin_chameleon")

add_definitions(-DTARGET_NAME=\\"${TARGET_NAME}\\")

find_package(Qt5 CONFIG REQUIRED COMPONENTS Core)
find_package(Qt5X11Extras REQUIRED)
find_package(KF5CoreAddons REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(X11 REQUIRED)
find_package(KF5Config REQUIRED)
find_package(KF5WindowSystem REQUIRED)
find_package(KDecoration2 REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
include_directories(${Qt5Core_PRIVATE_INCLUDE_DIRS})
include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(
    ${TARGET_NAME}
    MODULE
    main.cpp
    chameleon.cpp
    chameleontheme.cpp
    chameleonbutton.cpp
    chameleonshadow.cpp
    chameleonconfig.cpp
    chameleonwindowtheme.cpp
    kwinutils.cpp
    themes.qrc
)
set_target_properties(${TARGET_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/org.kde.kdecoration2/")

target_link_libraries(
    ${TARGET_NAME}
    PUBLIC
    Qt5::Core
    Qt5::DBus
    Qt5::X11Extras
    Qt5::Qml
    X11
    XCB::XCB
    kwineffects
    KDecoration2::KDecoration
    KF5::CoreAddons
    KF5::ConfigCore
    KF5::WindowSystem
    kwin
)

install(TARGETS ${TARGET_NAME} DESTINATION ${KDE_INSTALL_PLUGINDIR}/org.kde.kdecoration2)
