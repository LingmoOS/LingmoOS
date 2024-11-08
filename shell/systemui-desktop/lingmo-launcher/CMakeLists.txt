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
find_package(Qt5 REQUIRED ${QT})
find_package(KF5WindowSystem REQUIRED)

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

qt5_add_dbus_adaptor(DBUS_SRCS
    src/com.lingmo.Launcher.xml
    src/launcher.h
    Launcher
    launcheradaptor
    LauncherAdaptor
)
set_source_files_properties(${DBUS_SRCS} PROPERTIES SKIP_AUTOGEN ON)

add_executable(${PROJECT_NAME} ${SRCS} ${DBUS_SRCS} ${RESOURCES})
target_link_libraries(${PROJECT_NAME}
        Qt5::Core
        Qt5::Widgets
        Qt5::DBus
        Qt5::Quick
        Qt5::QuickControls2
        KF5::WindowSystem
)

file(GLOB TS_FILES translations/*.ts)
foreach(filepath ${TS_FILES})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath})
    list(APPEND ts_files_replaced ${filename})
endforeach()
qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${ts_files_replaced})
add_custom_target(translations ALL DEPENDS ${QM_FILES} SOURCES ${ts_files_replaced})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${QM_FILES} DESTINATION /usr/share/${PROJECT_NAME}/translations)
