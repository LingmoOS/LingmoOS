cmake_minimum_required(VERSION 3.10)

set(DTK_VERSION "5.6.23" CACHE STRING "Define project version")
project(DtkDeclarative
    VERSION "${DTK_VERSION}"
    DESCRIPTION "DTK Declarative module"
    HOMEPAGE_URL "https://github.com/linuxdeepin/dtkdeclarative"
    LANGUAGES CXX
)

if (${PROJECT_VERSION_MAJOR} STREQUAL "5")
    set(QT_DEFAULT_MAJOR_VERSION "5")
    set(QT_VERSION_MAJOR "5")
    set(EnableQt5 on)
    set(EnableDtk5 on)
    set(EnableQt6 off)
    set(EnableDtk6 off)
elseif(${PROJECT_VERSION_MAJOR} STREQUAL "6")
    set(QT_DEFAULT_MAJOR_VERSION "6")
    set(QT_VERSION_MAJOR "6")
    set(DTK_VERSION_MAJOR "6")
    set(EnableQt5 off)
    set(EnableDtk5 off)
    set(EnableQt6 on)
    set(EnableDtk6 on)
else()
    message(FATAL_ERROR "Only support DTK_VERSION is 5 or 6")
endif()

set(ENABLE_COV OFF CACHE BOOL "Generate coverage info")
set(LIB_NAME dtk${DTK_VERSION_MAJOR}declarative)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif()
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

include(DtkBuildConfig)
set(BUILD_DOCS ON CACHE BOOL "Generate doxygen-based documentation")

set(LIB_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}" CACHE STRING "Library install path")
set(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_INCLUDEDIR}/dtk${PROJECT_VERSION_MAJOR}/DDeclarative" CACHE STRING "Headers install path")
set(TEMPLATE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/qtcreator/templates/wizards/projects/qml${DTK_VERSION_MAJOR}-app-template" CACHE STRING "Directory to install QtCreator template")
set(CONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/Dtk${DTK_VERSION_MAJOR}Declarative" CACHE STRING "CMake config file install directory")
set(PKGCONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/pkgconfig" CACHE STRING "Directory to install pkg-config file")
set(MKSPECS_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/qt${QT_VERSION_MAJOR}/mkspecs/modules" CACHE STRING "Qt pri module install directory")
set(QML_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/qt${QT_VERSION_MAJOR}/qml" CACHE STRING "Qml plugin install directory")

set(USE_QQuickStylePluginPrivate OFF)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR}QuickControls2)
if(EnableQt5)
if(TARGET Qt::QuickControls2 AND TARGET Qt::QuickControls2Private)
    set(USE_QQuickStylePluginPrivate ON)
endif()
endif()
if(EnableQt6)
if(TARGET Qt6::QuickControls2 AND TARGET Qt6::QuickControls2Private)
    set(USE_QQuickStylePluginPrivate ON)
endif()
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall -Wextra")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--as-needed")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    # 加上 ASAN 检查后可能会导致 DEBUG 应用启动后退出。可以加上 ASAN_OPTIONS 环境变量来防止应用退出
    # ASAN_OPTIONS="halt_on_error=0" ASAN_OPTIONS="new_delete_type_mismatch=0"
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fno-omit-frame-pointer")
    set(BUILD_TESTING ON)
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")
endif ()

set(DDECLARATIVE_TRANSLATIONS_DIR "dtk${PROJECT_VERSION_MAJOR}/DDeclarative/translations" CACHE STRING "DDeclarative translations directory")
set(DDECLARATIVE_TRANSLATIONS_PATH "share/${DDECLARATIVE_TRANSLATIONS_DIR}")
set(TRANSLATIONS_INSTALL_PATH "${DDECLARATIVE_TRANSLATIONS_PATH}")

set(URI "org.deepin.dtk")
string(REPLACE "." "/" URI_PATH ${URI})
set(PLUGIN_NAME dtkdeclarativeplugin)
set(STYLE_PLUGIN_NAME qtquickcontrolschameleonstyleplugin)
set(PLUGIN_OUTPUT_DIR ${PROJECT_BINARY_DIR}/plugins)

if(EnableQt5)
    add_subdirectory(src)
    add_subdirectory(qmlplugin)
endif()
if(EnableQt6)
    add_subdirectory(qt6)
endif()
add_subdirectory(chameleon)
add_subdirectory(examples)

if(BUILD_DOCS)
    add_subdirectory(docs)
endif()

if(BUILD_TESTING)
    add_subdirectory(tests)
    add_dependencies(unit-test ${PLUGIN_NAME} ${STYLE_PLUGIN_NAME})
    if(EnableQt6)
        add_dependencies(unit-test dtkdeclarativeprivatesplugin dtkdeclarativesettingsplugin)
    endif()
endif()

# Install wizards template
set(QML_TEMPLATE_QTVERSION_INDEX 0)
if(EnableQt5)
    set(QML_TEMPLATE_QTVERSION_INDEX 3)
endif()
configure_package_config_file(
    "${CMAKE_CURRENT_LIST_DIR}/misc/qml-app-template/wizard.json.in"
    "${CMAKE_CURRENT_BINARY_DIR}/wizard.json"
    INSTALL_DESTINATION "${TEMPLATE_INSTALL_DIR}"
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/wizard.json" DESTINATION "${TEMPLATE_INSTALL_DIR}")
install(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/misc/qml-app-template/" DESTINATION "${TEMPLATE_INSTALL_DIR}" FILES_MATCHING PATTERN "*" PATTERN "*.in" EXCLUDE)

configure_package_config_file(
    "${CMAKE_CURRENT_LIST_DIR}/misc/DtkDeclarativeConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/Dtk${DTK_VERSION_MAJOR}DeclarativeConfig.cmake"
    INSTALL_DESTINATION "${CONFIG_INSTALL_DIR}"
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/Dtk${DTK_VERSION_MAJOR}DeclarativeConfigVersion.cmake"
    VERSION ${VERSION}
    COMPATIBILITY SameMajorVersion
)
# Install cmake config file
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/Dtk${DTK_VERSION_MAJOR}DeclarativeConfig.cmake" DESTINATION "${CONFIG_INSTALL_DIR}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/Dtk${DTK_VERSION_MAJOR}DeclarativeConfigVersion.cmake" DESTINATION "${CONFIG_INSTALL_DIR}")
# Install pkg-config file
configure_file("${PROJECT_SOURCE_DIR}/misc/dtkdeclarative.pc.in" "${CMAKE_CURRENT_BINARY_DIR}/dtk${DTK_VERSION_MAJOR}declarative.pc" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/dtk${DTK_VERSION_MAJOR}declarative.pc" DESTINATION "${PKGCONFIG_INSTALL_DIR}")
# Install qmake module config file
configure_file("${CMAKE_CURRENT_LIST_DIR}/misc/qt_lib_dtkdeclarative.pri.in" "${CMAKE_CURRENT_BINARY_DIR}/qt_lib_dtkdeclarative.pri" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/qt_lib_dtkdeclarative.pri" DESTINATION "${MKSPECS_INSTALL_DIR}")
