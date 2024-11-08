cmake_minimum_required(VERSION 3.5)

project(top  LANGUAGES CXX C)
add_subdirectory(src)
add_subdirectory(uNoteDbusServer)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")

