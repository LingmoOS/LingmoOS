cmake_minimum_required(VERSION 3.13)

project(search_client VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Core Gui REQUIRED)
find_package(Dtk COMPONENTS Widget Core Gui REQUIRED)

file(GLOB PROJECT_SOURCES CONFIGURE_DEPENDS ./*.cpp ./*.h)

add_executable(
    ${PROJECT_NAME}
    ${PROJECT_SOURCES}
)
target_include_directories(${PROJECT_NAME} PUBLIC include)

target_link_libraries(
    ${PROJECT_NAME} PRIVATE
    ${DtkWidget_LIBRARIES}
    ${DtkCore_LIBRARIES}
    dtksearch
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Core
)

