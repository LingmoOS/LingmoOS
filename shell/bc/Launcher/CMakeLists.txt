cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME lingmo-launcher)
project(${PROJECT_NAME})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT Core Widgets DBus Quick QuickControls2 LinguistTools)
find_package(Qt6 REQUIRED ${QT})
find_package(Qt6 REQUIRED COMPONENTS Core5Compat)
find_package(KF6WindowSystem REQUIRED)

set(SRCS
    src/desktopproperties.cpp
    src/iconthemeimageprovider.cpp
    src/launcher.cpp
    src/launchermodel.cpp
    src/appitem.cpp
    src/main.cpp
    src/pagemodel.cpp
    src/ucunits.cpp
    src/listmodelmanager.cpp
    src/iconitem.cpp
    src/processprovider.cpp
    src/appmanager.cpp
)

set(RESOURCES
    qml.qrc
)

qt_add_dbus_adaptor(DBUS_SRCS
    src/com.lingmo.Launcher.xml
    src/launcher.h
    Launcher
    launcheradaptor
    LauncherAdaptor
)
set_source_files_properties(${DBUS_SRCS} PROPERTIES SKIP_AUTOGEN ON)

add_executable(${PROJECT_NAME} ${SRCS} ${DBUS_SRCS} ${RESOURCES})
target_link_libraries(${PROJECT_NAME}
        Qt6::Core
        Qt6::Widgets
        Qt6::DBus
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Core5Compat
        KF6::WindowSystem
)

file(GLOB TS_FILES translations/*.ts)
foreach(filepath ${TS_FILES})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath})
    list(APPEND ts_files_replaced ${filename})
endforeach()
qt6_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${ts_files_replaced})
add_custom_target(translations ALL DEPENDS ${QM_FILES} SOURCES ${ts_files_replaced})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${QM_FILES} DESTINATION /usr/share/${PROJECT_NAME}/translations)
