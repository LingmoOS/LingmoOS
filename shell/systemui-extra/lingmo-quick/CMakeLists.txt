cmake_minimum_required(VERSION 3.16)
project(lingmo-quick)
set(ROOT_PROJECT_NAME lingmo-quick)
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_MICRO 0)
set(LINGMO_QUICK_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})

include(CMakePackageConfigHelpers)
add_subdirectory(core)
add_subdirectory(items)
add_subdirectory(platform)
add_subdirectory(framework)
add_subdirectory(modules)
add_subdirectory(widgets)

set(CMAKE_CONFIG_INSTALL_DIR "/usr/share/cmake/lingmo-quick")
configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/lingmo-quick-config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/lingmo-quick-config.cmake
        INSTALL_DESTINATION  ${CMAKE_CONFIG_INSTALL_DIR}
)

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/lingmo-quick-config-version.cmake
        VERSION ${LINGMO_QUICK_VERSION}
        COMPATIBILITY SameMajorVersion
)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/lingmo-quick-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/lingmo-quick-config-version.cmake
        DESTINATION ${CMAKE_CONFIG_INSTALL_DIR}
        COMPONENT Devel
)
