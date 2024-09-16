cmake_minimum_required(VERSION 3.13)

project(burn_client VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)
find_package(Dtk COMPONENTS Core REQUIRED)
#find_package(dtkburn REQUIRED)

file(GLOB PROJECT_SOURCES CONFIGURE_DEPENDS ./*.cpp ./*.h)

add_executable(
    ${PROJECT_NAME}
    ${PROJECT_SOURCES}
)
target_include_directories(${PROJECT_NAME} PUBLIC include)

target_link_libraries(
    ${PROJECT_NAME} PRIVATE
    ${DtkCore_LIBRARIES}
    ${dtkburn_LIBRARIES}
    dtkburn
)
