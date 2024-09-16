# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

set(BIN_NAME dtkcompressor)

file(GLOB_RECURSE INCLUDE_FILES "${PROJECT_SOURCE_DIR}/include/${BIN_NAME}/*")
file(GLOB_RECURSE SRCS
    "./archivemanager/*.h"
    "./archivemanager/*.cpp"
)

add_library(${BIN_NAME} SHARED
    ${INCLUDE_FILES}
    ${SRCS}
)

## export include path
if(NOT DEFINED HFILES_OUTPUT_PATH)
    set(HFILES_OUTPUT_PATH ${CMAKE_BINARY_DIR}/include)
endif()
add_subdirectory(3rdparty)
find_package(KF5Codecs REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(DtkCore REQUIRED)
if(QT_VERSION_MAJOR EQUAL "6")
    find_package(Qt6Core5Compat REQUIRED)
endif()

target_include_directories(${BIN_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/${BIN_NAME}/)
target_include_directories(${BIN_NAME} PUBLIC ${KF5Codecs_INCLUDE_DIRS})

set_target_properties(${BIN_NAME} PROPERTIES
    VERSION ${CMAKE_PROJECT_VERSION}
    SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR})

target_compile_definitions(${BIN_NAME} PRIVATE VERSION="${CMAKE_PROJECT_VERSION}")

if(QT_VERSION_MAJOR EQUAL "6")
    target_link_libraries(${BIN_NAME} PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt6::Core5Compat
        ${DtkCore_LIBRARIES}
        interfacecompressor
        KF5::Codecs
    )
else()
    target_link_libraries(${BIN_NAME} PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        ${DtkCore_LIBRARIES}
        interfacecompressor
        KF5::Codecs
    )
endif()


target_include_directories(${BIN_NAME} PUBLIC
    ${HFILES_OUTPUT_PATH}
)

install(FILES ${INCLUDE_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${BIN_NAME})
install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})

# for pc file config
set(PC_LIBS_PRIVATE Qt${QT_VERSION_MAJOR}Core)
set(PC_REQ_PRIVATE)
set(PC_REQ_PUBLIC)

# config pkgconfig file
configure_file(${PROJECT_SOURCE_DIR}/misc/${BIN_NAME}/${BIN_NAME}.pc.in ${BIN_NAME}.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${BIN_NAME}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

# config cmake file
configure_file(${PROJECT_SOURCE_DIR}/misc/${BIN_NAME}/${BIN_NAME}Config.cmake.in ${BIN_NAME}Config.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${BIN_NAME}Config.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${BIN_NAME})

# config qmake moudule file
set(DTK_MODULE ${BIN_NAME})
set(DTK_DEPS "core")
set(QMKSPECS_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/qt${QT_VERSION_MAJOR}/mkspecs/modules" CACHE STRING "INSTALL DIR FOR qt pri files")
configure_file(${PROJECT_SOURCE_DIR}/misc/${BIN_NAME}/qt_lib_${BIN_NAME}.pri.in qt_lib_${BIN_NAME}.pri @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qt_lib_${BIN_NAME}.pri DESTINATION "${QMKSPECS_INSTALL_DIR}")

## plugin install
if(NOT DEFINED DTK_COMPRESSOR_PLUGIN_PATH)
    set(DTK_COMPRESSOR_PLUGIN_PATH ${CMAKE_INSTALL_LIBDIR}/dtkcompressor/plugins)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(BUILD_TESTING ON)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -Wl,-rpath=${LIBRARY_OUTPUT_PATH}")
else()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wl,-O1,-rpath=${CMAKE_INSTALL_PREFIX}/${DTK_COMPRESSOR_PLUGIN_PATH}")
endif ()

install(TARGETS cli7zplugincompressor DESTINATION ${DTK_COMPRESSOR_PLUGIN_PATH})
install(TARGETS clirarplugincompressor DESTINATION ${DTK_COMPRESSOR_PLUGIN_PATH})
install(TARGETS zipplugincompressor DESTINATION ${DTK_COMPRESSOR_PLUGIN_PATH})
install(TARGETS pigzplugincompressor DESTINATION ${DTK_COMPRESSOR_PLUGIN_PATH})
install(TARGETS minizipplugincompressor DESTINATION ${DTK_COMPRESSOR_PLUGIN_PATH})
install(TARGETS readwritelibarchiveplugincompressor DESTINATION ${DTK_COMPRESSOR_PLUGIN_PATH})
install(TARGETS readonlylibarchiveplugincompressor DESTINATION ${DTK_COMPRESSOR_PLUGIN_PATH})
install(TARGETS interfacecompressor DESTINATION ${DTK_COMPRESSOR_PLUGIN_PATH})
install(TARGETS bz2plugincompressor DESTINATION ${DTK_COMPRESSOR_PLUGIN_PATH})
install(TARGETS gzplugincompressor DESTINATION ${DTK_COMPRESSOR_PLUGIN_PATH})
install(TARGETS xzplugincompressor DESTINATION ${DTK_COMPRESSOR_PLUGIN_PATH})
