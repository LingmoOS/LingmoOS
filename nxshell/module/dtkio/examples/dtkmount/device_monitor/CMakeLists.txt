cmake_minimum_required(VERSION 3.13)

project(device_monitor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Core Gui DBus LinguistTools REQUIRED)
find_package(Dtk COMPONENTS Widget Core Gui REQUIRED)
#find_package(dtkmount REQUIRED)

file(GLOB TS_FILES CONFIGURE_DEPENDS translations/*.ts)
file(GLOB PROJECT_SOURCES CONFIGURE_DEPENDS src/*.cpp include/*.h ${TS_FILES})

if(QT_VERSION_MAJOR EQUAL "6")
    qt_create_translation(
        QM_FILES
        ${PROJECT_SOURCES}
        ${TS_FILES}
    )
else()
    qt5_create_translation(
        QM_FILES
        ${PROJECT_SOURCES}
        ${TS_FILES}
    )
endif()

add_executable(
    ${PROJECT_NAME}
    ${QM_FILES} 
    ${PROJECT_SOURCES}
)

target_include_directories(${PROJECT_NAME} PUBLIC include)

target_link_libraries(
    ${PROJECT_NAME} PRIVATE
    ${DtkWidget_LIBRARIES}
    ${DtkCore_LIBRARIES}
    dtkmount
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
)
