cmake_minimum_required(VERSION 3.5)

set(LINGMOUI_VERSION 1.0)
project(lingmoui VERSION ${LINGMOUI_VERSION})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#DEFAULT COMPONENTS DEFINITIONS
option(COMPONENT_EDITOR "Build editor component" ON)
option(COMPONENT_FM "Build filemanager component" ON)
option(COMPONENT_ACCOUNTS "Build accounts component" ON)
option(COMPONENT_TERMINAL "Build terminal component" ON)
option(COMPONENT_STORE "Build store component" OFF)
option(COMPONENT_TAGGING "Build tagging component" ON)
option(COMPONENT_SYNCING "Build syncing component" ON)

find_package(ECM 5.45.0 NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

set(QT Core Widgets Quick QuickControls2 DBus)
if (UNIX)
    if (NOT APPLE)
        set(QT ${QT} X11Extras)
    endif ()
endif ()
find_package(Qt5 REQUIRED ${QT})
if (UNIX)
    if (NOT APPLE)
        find_package(KF5WindowSystem REQUIRED)
    endif ()
endif ()

include(GenerateExportHeader)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(ECMGenerateHeaders)

# Get the installation directory from qmake
get_target_property(QT_QMAKE_EXECUTABLE ${Qt5Core_QMAKE_EXECUTABLE} IMPORTED_LOCATION)
if (NOT QT_QMAKE_EXECUTABLE)
    message(FATAL_ERROR "qmake is not found.")
endif ()

execute_process(COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_QML
        OUTPUT_VARIABLE INSTALL_QMLDIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
if (INSTALL_QMLDIR)
    message(STATUS "qml directory:" "${INSTALL_QMLDIR}")
else ()
    message(FATAL_ERROR "qml directory cannot be detected.")
endif ()

# Install
if (MSVC)
    # Install LingmoUI to vcpkg installed dir
    # for example.
    # _VCPKG_INSTALLED_DIR:PATH=C:/app/vcpkg/installed
    # VCPKG_TARGET_TRIPLET:STRING=x64-windows
    # The final installed dir is C:/app/vcpkg/installed/x64-windows/share/LingmoUI
    set(CMAKECONFIG_INSTALL_DIR "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share/LingmoUI")
    set(CMAKE_INSTALL_PREFIX "${_VCPKG_INSTALLED_DIR}")
else ()
    set(CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/LingmoUI")
endif ()
message(STATUS "install directory:" "${CMAKECONFIG_INSTALL_DIR}")

# Qt Widgets Style
#if (UNIX)
#    find_package(ECM REQUIRED NO_MODULE)
#    set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
#    include(ECMQueryQmake)
#    query_qmake(CMAKE_INSTALL_QTPLUGINDIR QT_INSTALL_PLUGINS)
#    add_subdirectory(qtwidgets-style)
#endif ()

add_subdirectory(src)

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/LingmoUIConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/LingmoUIConfig.cmake"
        INSTALL_DESTINATION
        "${CMAKECONFIG_INSTALL_DIR}"
        PATH_VARS
        CMAKECONFIG_INSTALL_DIR CMAKE_INSTALL_PREFIX
)

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/LingmoUIConfigVersion.cmake
        VERSION ${LIB_VERSION}
        COMPATIBILITY AnyNewerVersion
)
if(NOT APPLE)
install(
        FILES
        "${CMAKE_CURRENT_BINARY_DIR}/LingmoUIConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/LingmoUIConfigVersion.cmake"
        DESTINATION
        "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT
        Devel
)
install(FILES conf/version DESTINATION /etc/LingmoUI/ COMPONENT Runtime)

# install(FILES src/Windows.tar.xz DESTINATION ~/.update/ COMPONENT Runtime)

install(EXPORT LingmoUITargets
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE LingmoUITargets.cmake
        )
endif()
