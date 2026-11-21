cmake_minimum_required(VERSION 3.16)

project(qsvg-rsvg)

set(TARGET qsvg-rsvg)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBRSVG REQUIRED IMPORTED_TARGET librsvg-2.0)

set(SRCS
    rsvgimageiohandler.cpp
    rsvgimageiohandler.h
    rsvgimageplugin.cpp
    rsvgimageplugin.h
)

add_library(${TARGET} MODULE ${SRCS})
target_compile_definitions(${TARGET} PRIVATE QT_NO_KEYWORDS)
set_target_properties(${TARGET} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/imageformats"
)
target_link_libraries(${TARGET}
    Qt6::Core
    Qt6::Gui
    PkgConfig::LIBRSVG
)

find_program(QT_PATHS_EXECUTABLE NAMES qtpaths6 REQUIRED)
execute_process(COMMAND "${QT_PATHS_EXECUTABLE}" -query QT_INSTALL_PLUGINS
    OUTPUT_VARIABLE CMAKE_INSTALL_QTPLUGINDIR OUTPUT_STRIP_TRAILING_WHITESPACE)

install(TARGETS ${TARGET} DESTINATION ${CMAKE_INSTALL_QTPLUGINDIR}/imageformats/)

# HEIF/HEIC/AVIF and JPEG XL, each built when its library is there.
add_subdirectory(heif)
add_subdirectory(jxl)
