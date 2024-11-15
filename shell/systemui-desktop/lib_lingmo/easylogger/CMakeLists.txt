cmake_minimum_required(VERSION 3.10)

project(LingmoLogger CXX C)

set(LIB_VERSION_MAJOR 1)
set(LIB_VERSION_MINOR 0)
set(LIB_VERSION_PATCH 0)
set(LIB_VERSION ${LIB_VERSION_MAJOR}.${LIB_VERSION_MINOR}.${LIB_VERSION_PATCH})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

file(GLOB SRCS_R "*.h" "*.c" "*.cpp")
foreach(filepath ${SRCS_R})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath})
    list(APPEND SRCS ${filename})
endforeach()

add_library(LingmoLogger SHARED ${SRCS})

list(APPEND PUBLIC_HEADER "elog.h" "elog_cfg.h")
set_target_properties(LingmoLogger  PROPERTIES 
        PUBLIC_HEADER "${PUBLIC_HEADER}"
        VERSION ${LIB_VERSION}
        SOVERSION ${LIB_VERSION_MAJOR}
)

generate_export_header(LingmoLogger BASE_NAME LingmoLogger)

install(TARGETS LingmoLogger
        EXPORT  LingmoLoggerTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        PUBLIC_HEADER DESTINATION include
)


set(CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/LingmoLogger")

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/LingmoLoggerConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/LingmoLoggerConfig.cmake"
        INSTALL_DESTINATION
        "${CMAKECONFIG_INSTALL_DIR}"
        PATH_VARS
        CMAKECONFIG_INSTALL_DIR CMAKE_INSTALL_PREFIX
)

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/LingmoLoggerConfigVersion.cmake
        VERSION ${LIB_VERSION}
        COMPATIBILITY AnyNewerVersion
)

install(
        FILES
        "${CMAKE_CURRENT_BINARY_DIR}/LingmoLoggerConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/LingmoLoggerConfigVersion.cmake"
        DESTINATION
        "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT
        Devel
)

install(EXPORT LingmoLoggerTargets
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE LingmoLoggerTargets.cmake
)

add_subdirectory(qml_plugin)