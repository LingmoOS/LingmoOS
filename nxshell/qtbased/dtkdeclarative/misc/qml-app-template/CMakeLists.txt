cmake_minimum_required(VERSION 3.1)

project(%{ProjectName} LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt%{QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
if(${QT_VERSION_MAJOR} EQUAL 5)
    set(DTK_VERSION_MAJOR "")
else()
    set(DTK_VERSION_MAJOR ${QT_VERSION_MAJOR})
endif()

find_package(Dtk${DTK_VERSION_MAJOR}Declarative REQUIRED)

# Application
add_subdirectory(src)
