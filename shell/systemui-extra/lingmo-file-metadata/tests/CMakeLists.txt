remove_definitions(-DQT_NO_CAST_FROM_ASCII)

add_executable(dump dump.cpp)

target_include_directories(dump  PUBLIC "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>/src")

target_link_libraries(dump
    Qt${QT_MAJOR_VERSION}::Core
    lingmo-file-metadata
)
