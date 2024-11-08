cmake_minimum_required(VERSION 3.16)
project(platform)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FeatureSummary)
find_package(ECM 5.92.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/Modules)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Quick Qml X11Extras Widgets DBus REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Quick Qml X11Extras Widgets DBus REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(KF5WindowSystem REQUIRED)
find_package(KF5Wayland)
find_package(WaylandScanner)
find_package(WaylandProtocols 1.15)
set_package_properties(WaylandProtocols PROPERTIES TYPE REQUIRED)
find_package(LingmoWaylandProtocols 1.0.0 CONFIG)
set_package_properties(LingmoWaylandProtocols PROPERTIES TYPE REQUIRED)
find_package(PlasmaWaylandProtocols 1.4.0 CONFIG)
set_package_properties(PlasmaWaylandProtocols PROPERTIES TYPE REQUIRED)

set(EXTERNAL_LIBS "")
set(PC_PKGS lingmosdk-waylandhelper gsettings-qt x11 Qt5Xdg)
foreach(external_lib IN ITEMS ${PC_PKGS})
    pkg_check_modules(${external_lib} REQUIRED IMPORTED_TARGET ${external_lib})
    if(${${external_lib}_FOUND})
        include_directories(${${external_lib}_INCLUDE_DIRS})
        link_directories(${${external_lib}_LIBRARY_DIRS})
        list(APPEND EXTERNAL_LIBS PkgConfig::${external_lib})
    endif()
endforeach()

set(LINGMO_QUICK_PLATFORM_SRCS
        lingmo/settings.cpp
        lingmo/app-launcher.cpp
        lingmo/lingmo-theme-proxy.cpp
        lingmo/screen-area-utils.cpp
        wayland/registry.cpp
        wayland/wayland-pointer_p.h
        wayland/wayland-integration.cpp
        windows/dialog.cpp
        windows/lingmo-window.cpp
        window-helper.cpp
        wm-interface.cpp
        wm-impl-x11.h wm-impl-x11.cpp
        wm-impl-wayland.cpp wm-impl-wayland.h
        abstract-window-manager.cpp
        wayland-window-manager.cpp wayland-window-manager.h
        window-manager.cpp
        xcb-window-manager.cpp xcb-window-manager.h
)
set(HEADERS
        lingmo/settings.h
        lingmo/app-launcher.h
        lingmo/lingmo-theme-proxy.h
        lingmo/screen-area-utils.h
        window-type.h
        wm-interface.h
        window-helper.h
        wayland/registry.h
        windows/dialog.h
        windows/lingmo-window.h
        window-manager.h
        abstract-window-manager.h
)
include_directories(wayland)
ecm_add_wayland_client_protocol(LINGMO_QUICK_PLATFORM_SRCS
        PROTOCOL ${LINGMO_WAYLAND_PROTOCOLS_DIR}/lingmo-shell.xml
        BASENAME lingmo-shell
)

ecm_add_wayland_client_protocol(LINGMO_QUICK_PLATFORM_SRCS
        PROTOCOL ${LINGMO_WAYLAND_PROTOCOLS_DIR}/lingmo-window-management.xml
        BASENAME lingmo-window-management
)

ecm_add_wayland_client_protocol(LINGMO_QUICK_PLATFORM_SRCS
        PROTOCOL ${PLASMA_WAYLAND_PROTOCOLS_DIR}/plasma-virtual-desktop.xml
        BASENAME plasma-virtual-desktop
)

add_library(${PROJECT_NAME} SHARED
        ${LINGMO_QUICK_PLATFORM_SRCS} ${HEADERS}
)
add_library(${ROOT_PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::X11Extras
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::DBus
        KF5::WindowSystem
	KF5::WaylandClient
        ${EXTERNAL_LIBS}
)

set(HEADERS_INSTALL_DIR /usr/include/lingmo-quick/platform)
target_include_directories(${PROJECT_NAME} INTERFACE $<INSTALL_INTERFACE:${HEADERS_INSTALL_DIR}>)

include(CMakePackageConfigHelpers)
set(CMAKE_CONFIG_INSTALL_DIR "/usr/share/cmake/lingmo-quick-platform")
set(PC_INSTALL_DIR "/usr/lib/pkgconfig")

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/lingmo-quick-platform-config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/lingmo-quick-platform-config.cmake"
        INSTALL_DESTINATION  ${CMAKE_CONFIG_INSTALL_DIR}
)

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/lingmo-quick-platform-config-version.cmake
        VERSION ${LINGMO_QUICK_VERSION}
        COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/lingmo-quick-platform.pc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/lingmo-quick-platform.pc"
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
        FILE lingmo-quick-platform-targets.cmake
        NAMESPACE ${ROOT_PROJECT_NAME}::
        DESTINATION ${CMAKE_CONFIG_INSTALL_DIR})

install(FILES ${HEADERS} DESTINATION ${HEADERS_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lingmo-quick-platform.pc DESTINATION ${PC_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lingmo-quick-platform-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/lingmo-quick-platform-config-version.cmake
        DESTINATION ${CMAKE_CONFIG_INSTALL_DIR})

set(LINGMO_QUICK_PLATFORM_PLUGIN_SRCS
        lingmo-quick-platform-plugin.cpp
        lingmo-quick-platform-plugin.h
)
add_library(${ROOT_PROJECT_NAME}-${PROJECT_NAME}-plugin SHARED ${LINGMO_QUICK_PLATFORM_PLUGIN_SRCS})

target_compile_definitions(${ROOT_PROJECT_NAME}-${PROJECT_NAME}-plugin PRIVATE
        PLUGIN_IMPORT_URI="org.lingmo.quick.platform"
        PLUGIN_VERSION_MAJOR=${VERSION_MAJOR}
        PLUGIN_VERSION_MINOR=${VERSION_MINOR}
)
target_link_libraries(${ROOT_PROJECT_NAME}-${PROJECT_NAME}-plugin
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Quick
        ${PROJECT_NAME}
)
set(PLUGIN_INSTALL_ROOT_PATH "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt5/qml/org/lingmo/quick/platform")
install(FILES qmldir DESTINATION "${PLUGIN_INSTALL_ROOT_PATH}")
install(TARGETS ${ROOT_PROJECT_NAME}-${PROJECT_NAME}-plugin LIBRARY DESTINATION "${PLUGIN_INSTALL_ROOT_PATH}")

enable_testing()
if(BUILD_TEST)
    add_subdirectory(test)
    add_subdirectory(autotest)
endif ()
