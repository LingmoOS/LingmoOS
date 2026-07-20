cmake_minimum_required(VERSION 3.14)

project(lingmo-clipboard LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(lingmo-clipboard
  main.cpp
  clipboard.cpp
)
target_link_libraries(lingmo-clipboard
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

install(TARGETS lingmo-clipboard DESTINATION ${CMAKE_INSTALL_BINDIR})