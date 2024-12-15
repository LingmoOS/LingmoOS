message(STATUS "Current Qt Version: ${QT_VERSION_MAJOR}")

set(LIB_NAME dtk${DTK_VERSION_MAJOR}gui)

# Set install path
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set build option
option(DTK_DISABLE_LIBXDG "Disable libxdg" OFF)
find_package(Qt${QT_VERSION_MAJOR}XdgIconLoader)
if (NOT Qt${QT_VERSION_MAJOR}XdgIconLoader_FOUND)
    message(WARNING " XdgIconLoader Not Found, DISABLE LIBXDG !")
    set (DTK_DISABLE_LIBXDG ON)
endif()
option(DTK_DISABLE_LIBRSVG "Disable librsvg" OFF)
option(DTK_DISABLE_EX_IMAGE_FORMAT "Disable libraw and freeimage" OFF)

set(CMAKE_CXX_STANDARD 17)

if("${QT_VERSION_MAJOR}" STREQUAL "5")
  set (BUILD_DOCS ON CACHE BOOL "Generate doxygen-based documentation")
else()
  # dtk6 not build doc
  set (BUILD_DOCS OFF CACHE BOOL "Generate doxygen-based documentation")
endif()

set(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_INCLUDEDIR}/dtk${PROJECT_VERSION_MAJOR}/DGui")
set(TOOL_INSTALL_DIR "${CMAKE_INSTALL_LIBEXECDIR}/dtk${PROJECT_VERSION_MAJOR}/DGui/bin")
set(LIBRARY_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}")
set(MKSPECS_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/qt${QT_VERSION_MAJOR}/mkspecs/modules" CACHE STRING "Install dir for qt pri module files")
set(CONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/Dtk${DTK_VERSION_MAJOR}Gui")
set(PKGCONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall -Wextra -fopenmp")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--as-needed")
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -pie")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(BUILD_TESTING ON)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fno-omit-frame-pointer")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")
endif ()

set(CONFIGNAME include/global/dtkgui_config.h)
file(WRITE ${CONFIGNAME}
  "// This is an auto-generated header, please don't modify it.\n"
)
file(GLOB CONFIGSOURCE include/DtkGui/*)

foreach(FILENAME ${CONFIGSOURCE})
  get_filename_component(thefile ${FILENAME} NAME)
  file(APPEND ${CONFIGNAME} "#define DTKGUI_CLASS_${thefile}\n")
endforeach()

# Generate cmake config file
configure_package_config_file(misc/DtkGuiConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/Dtk${DTK_VERSION_MAJOR}GuiConfig.cmake
    INSTALL_DESTINATION "${CONFIG_INSTALL_DIR}"
    PATH_VARS TOOL_INSTALL_DIR
)
# Generate cmake version file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/Dtk${DTK_VERSION_MAJOR}GuiConfigVersion.cmake"
    VERSION ${DTK_VERSION}
    COMPATIBILITY SameMajorVersion
)
# Install cmake config file and version file to CONFIG_INSTALL_DIR
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/Dtk${DTK_VERSION_MAJOR}GuiConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/Dtk${DTK_VERSION_MAJOR}GuiConfigVersion.cmake
    DESTINATION "${CONFIG_INSTALL_DIR}"
)
# Install pkgconfig file
configure_file(misc/dtkgui.pc.in dtk${DTK_VERSION_MAJOR}gui.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/dtk${DTK_VERSION_MAJOR}gui.pc DESTINATION "${PKGCONFIG_INSTALL_DIR}")
# Install pri module
configure_file(misc/qt_lib_dtkgui.pri.in qt_lib_dtkgui.pri @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qt_lib_dtkgui.pri DESTINATION "${MKSPECS_INSTALL_DIR}")

set(GUISGNAME DtkGuis)

file(WRITE ${GUISGNAME}
  "// This is an auto-generated header, please don't modify it.\n"
  "#ifndef DTK_GUI_MODULE_H\n"
  "#define DTK_GUI_MODULE_H\n"
)
file(GLOB FILEGRAGTOWRITSOURCE include/filedrag/*)

foreach(FILENAME ${FILEGRAGTOWRITSOURCE})
  get_filename_component(thefile ${FILENAME} NAME)
  file(APPEND ${GUISGNAME} "#include \"${thefile}\"\n")
endforeach()
file(GLOB KERNELTOWRITESOURCE include/kernel/*)
foreach(FILENAME ${KERNELTOWRITESOURCE})
  get_filename_component(thefile ${FILENAME} NAME)
  file(APPEND ${GUISGNAME} "#include \"${thefile}\"\n")
endforeach()
file(GLOB UTILTOWRITESOURCE include/util/*)
foreach(FILENAME ${UTILTOWRITESOURCE})
  get_filename_component(thefile ${FILENAME} NAME)
  file(APPEND ${GUISGNAME} "#include \"${thefile}\"\n")
endforeach()
file(APPEND ${GUISGNAME} "#endif")
install(FILES DtkGuis DESTINATION "${INCLUDE_INSTALL_DIR}")

# Find common dependencies
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui DBus Network)
find_package(PkgConfig REQUIRED)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED Core)
find_package(DtkBuildHelper REQUIRED)
pkg_check_modules(librsvg REQUIRED IMPORTED_TARGET librsvg-2.0)

# Check optional image handler dependencies.
find_package(FreeImage)
pkg_check_modules(libraw IMPORTED_TARGET libraw)
if(FreeImage_FOUND AND libraw_FOUND)
  set(EX_IMAGE_FORMAT_LIBS_FOUND ON)
endif()

set(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
add_subdirectory(src ${OUTPUT_DIR}/src)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests ${OUTPUT_DIR}/tests)
endif()
add_subdirectory(tools  ${OUTPUT_DIR}/tools)
add_subdirectory(examples ${OUTPUT_DIR}/examples)

if (BUILD_DOCS)
  add_subdirectory(docs ${OUTPUT_DIR}/docs)
endif ()
