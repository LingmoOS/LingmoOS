cmake_minimum_required(VERSION 3.25)

add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x050F00)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(QT_QML_GENERATE_QMLLS_INI ON)

set(PROJECT_NAME LingmoUI)
set(MODULE_VERSION_MAJOR 3)
set(MODULE_VERSION_MINOR 1)
set(MODULE_VERSION_PATCH 1)
set(MODULE_VERSION ${MODULE_VERSION_MAJOR}.${MODULE_VERSION_MINOR}.${MODULE_VERSION_PATCH})
set(TARGET ${PROJECT_NAME}${MODULE_VERSION_MAJOR})

# Set this to allow indexing when we are using Qt Creator
set(QML_IMPORT_PATH ${CMAKE_BINARY_DIR}/temp CACHE STRING "" FORCE)

project(${PROJECT_NAME} VERSION ${MODULE_VERSION})

option(BUILD_PYTHON "Build Python bindings" OFF)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_DEFAULT_MAJOR_VERSION 6)
set(REQUIRED_QT_VERSION 6.5.0)

find_package(ECM REQUIRED NO_MODULE)

list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})
message(STATUS "ECM version: ${ECM_VERSION}")

set(QT Core Widgets Quick QuickControls2 DBus Core5Compat)
find_package(Qt6 ${REQUIRED_QT_VERSION} REQUIRED ${QT})
if (UNIX)
    if (NOT APPLE)
        list(APPEND QT GuiPrivate)
        find_package(KF6WindowSystem )
        # If not found, warn and continue
        if (NOT KF6WindowSystem_FOUND)
            message(WARNING "KF6WindowSystem not found. Some features may not work.")
        endif ()
    endif ()
endif ()

# Set Qt Policy
if(QT_KNOWN_POLICY_QTP0004)
    qt_policy(SET QTP0004 NEW)
endif()

# QHotkey
option(QHOTKEY_INSTALL OFF)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/QHotkey)
set(QHOTKEY_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/QHotkey/QHotkey)

include(GenerateExportHeader)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(ECMGenerateHeaders)

# Get the installation directory from qmake
if(ECM_VERSION VERSION_LESS 5.93.0)
    include(ECMQueryQmake)
    query_qmake(INSTALL_QMLDIR QT_INSTALL_QML)
else()
    include(ECMQueryQt)
    ecm_query_qt(INSTALL_QMLDIR QT_INSTALL_QML)
endif()

# Handle Python build status
if (BUILD_PYTHON)
    message(STATUS "Python bindings will be built")
    set(INSTALL_QMLDIR ${CMAKE_INSTALL_PREFIX}/qml)
    set(CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/${TARGET}")
endif ()

if (INSTALL_QMLDIR)
    message(STATUS "QML directory:" "${INSTALL_QMLDIR}")
else ()
    message(FATAL_ERROR "QML directory cannot be detected.")
endif ()

# Install
if (_VCPKG_INSTALLED_DIR)
    set(CMAKECONFIG_INSTALL_DIR "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share/${TARGET}")
    set(CMAKE_INSTALL_PREFIX "${_VCPKG_INSTALLED_DIR}")
else () 
    set(CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${TARGET}")
endif ()

message(STATUS "CMake Config install directory:" "${CMAKECONFIG_INSTALL_DIR}")

message(STATUS "LingmoUI Install Prefix:" "${CMAKE_INSTALL_PREFIX}")

# Set temp dir for import qml files in example app
set(LINGMOUI_TEMP_IMPORT_PATH ${CMAKE_BINARY_DIR}/temp/)

# Qt Widgets Style
#if (UNIX)
#    find_package(ECM REQUIRED NO_MODULE)
#    set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
#    include(ECMQueryQmake)
#    query_qmake(CMAKE_INSTALL_QTPLUGINDIR QT_INSTALL_PLUGINS)
#    add_subdirectory(qtwidgets-style)
#endif ()

add_subdirectory(src)

add_subdirectory(LingmoStyle)

# For compatible with old LingmoUI 1.0
add_subdirectory(Compatible)

add_subdirectory(example)

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/LingmoUIConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}Config.cmake"
        INSTALL_DESTINATION
        "${CMAKECONFIG_INSTALL_DIR}"
        PATH_VARS
        CMAKECONFIG_INSTALL_DIR CMAKE_INSTALL_PREFIX
)

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}ConfigVersion.cmake
        VERSION ${MODULE_VERSION}
        COMPATIBILITY AnyNewerVersion
)
if(NOT APPLE)
install(
        FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}ConfigVersion.cmake"
        DESTINATION
        "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT
        Devel
)
install(FILES conf/version DESTINATION ${CMAKE_INSTALL_PREFIX}/etc/${TARGET}/ COMPONENT Runtime)

install(EXPORT ${TARGET}Targets
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE LingmoUITargets.cmake
        )
endif()
