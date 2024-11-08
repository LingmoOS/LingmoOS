cmake_minimum_required(VERSION 3.14)
project(core)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Quick Qml REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Quick Qml REQUIRED)

set(LINGMO_QUICK_CORE_SRCS
        margin.cpp
        types.cpp
        shared-engine-view.cpp
        shared-qml-engine.cpp
        shared-engine-component.cpp
)
set(HEADERS
        margin.h
        types.h
        shared-engine-view.h
        shared-qml-engine.h
        shared-engine-component.h
)
add_library(${PROJECT_NAME} SHARED
        ${LINGMO_QUICK_CORE_SRCS}
)
add_library(lingmo-quick::core ALIAS ${PROJECT_NAME})
add_library(${ROOT_PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Qml
)

set(HEADERS_INSTALL_DIR /usr/include/lingmo-quick/core)
target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<INSTALL_INTERFACE:${HEADERS_INSTALL_DIR}>
)

include(CMakePackageConfigHelpers)
set(CMAKE_CONFIG_INSTALL_DIR "/usr/share/cmake/lingmo-quick-core")
set(PC_INSTALL_DIR "/usr/lib/pkgconfig")

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/lingmo-quick-core-config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/lingmo-quick-core-config.cmake"
        INSTALL_DESTINATION  ${CMAKE_CONFIG_INSTALL_DIR})
write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/lingmo-quick-core-config-version.cmake
        VERSION ${LINGMO_QUICK_VERSION}
        COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/lingmo-quick-core.pc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/lingmo-quick-core.pc"
        INSTALL_DESTINATION  ${PC_INSTALL_DIR})
set_target_properties(${PROJECT_NAME} PROPERTIES
        VERSION ${LINGMO_QUICK_VERSION}
        SOVERSION ${VERSION_MAJOR}
        OUTPUT_NAME ${ROOT_PROJECT_NAME}-${PROJECT_NAME}
)

install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}
        PUBLIC_HEADER DESTINATION ${HEADERS_INSTALL_DIR}
        LIBRARY DESTINATION /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}
)
install(EXPORT ${PROJECT_NAME}
        FILE lingmo-quick-core-targets.cmake
        NAMESPACE ${ROOT_PROJECT_NAME}::
        DESTINATION ${CMAKE_CONFIG_INSTALL_DIR})

install(FILES ${HEADERS} DESTINATION ${HEADERS_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lingmo-quick-core.pc DESTINATION ${PC_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lingmo-quick-core-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/lingmo-quick-core-config-version.cmake
        DESTINATION ${CMAKE_CONFIG_INSTALL_DIR})
