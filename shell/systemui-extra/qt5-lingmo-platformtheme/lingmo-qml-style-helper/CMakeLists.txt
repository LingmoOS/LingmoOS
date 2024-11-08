cmake_minimum_required(VERSION 3.16)

project(lingmo-qml-style-helper)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_MIN_VERSION "5.12.0")
set(KF5_MIN_VERSION "5.66.0")

find_package(Qt5Quick)
message(${imported_location})
set(LIB_PATH ${imported_location})
get_filename_component(PARENT_PATH "${LIB_PATH}" PATH)
message(1111)
message("lingmo-qml-style-helper PARENT_PATH: ${PARENT_PATH}")

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Widgets
)
find_package(KF5Kirigami2 CONFIG REQUIRED COMPONENTS)
find_package(KF5Config CONFIG REQUIRED COMPONENTS)

find_package(PkgConfig REQUIRED)
pkg_check_modules(Qsettings REQUIRED gsettings-qt)
include_directories(${Qsettings_INCLUDE_DIRS})

file(GLOB_RECURSE HEADER_FILES "*.h")
file(GLOB_RECURSE SRC_FILES "*.cpp")
file(GLOB_RECURSE OTHER_FILES qmldir)

SOURCE_GROUP("Header Files" FILES ${HEADER_FILES})
SOURCE_GROUP("Source Files" FILES ${SRC_FILES})
SOURCE_GROUP("other files" FILES ${OTHER_FILES})

include_directories(../libqt5-lingmo-style/)

add_library(lingmo-qml-style-helper MODULE ${HEADER_FILES} ${SRC_FILES} ${OTHER_FILES})

target_link_libraries(lingmo-qml-style-helper PRIVATE
    KF5::ConfigCore
    KF5::Kirigami2
    Qt5::Quick
    Qt5::Widgets
    gsettings-qt
    qt5-lingmo-style
    )

if(UNIX)
    set(TARGET_PATH "${PARENT_PATH}/qt5/qml/org/lingmo/qqc2style/private/")
    MESSAGE("liblingmo-qml-style-helper TARGET_PATH: ${TARGET_PATH}")

    install(TARGETS ${PROJECT_NAME} DESTINATION ${TARGET_PATH})
    install(FILES ${OTHER_FILES} DESTINATION "${TARGET_PATH}")
endif()
