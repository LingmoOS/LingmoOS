#cmake_minimum_required(VERSION 3.5)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
find_package(Dtk${DTK_VERSION_MAJOR}Log REQUIRED)
if(LINUX)
  find_package(PkgConfig REQUIRED)
  find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS DBus)
  if("${QT_VERSION_MAJOR}" STREQUAL "5")
      pkg_check_modules(QGSettings REQUIRED IMPORTED_TARGET gsettings-qt) #Dtk6 removed.
  endif()

endif()
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Xml)
find_package(DtkBuildHelper REQUIRED)

# start text encoding
find_package(ICU REQUIRED COMPONENTS uc)
pkg_check_modules(uchardet REQUIRED uchardet)
# end text encoding

# start base
include(base/base.cmake)
# end base

# start dci
include(dci/dci.cmake)
#end dci

#start filesystem
include(filesystem/filesystem.cmake)
#end filesystem
# start log
include(log/log.cmake)
#end log
# start settings
include(settings/settings.cmake)
#end settings

#start utils
include(util/util.cmake)
#end utils

#GLOB
include(glob.cmake)
#endGLOG
if(LINUX)
  add_library(${LIB_NAME} SHARED
    ${base_SRCS}
    ${dci_SRCS}
    ${filesystem_SRCS}
    ${log_SRCS}
    ${settings_SRC}
    ${utils_SRC}
    ${glob_SRC}
  )
  target_link_libraries(
    ${LIB_NAME} PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Xml
    Dtk${DTK_VERSION_MAJOR}::Log
  )
  target_link_libraries(${LIB_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::CorePrivate
  )
  if("${QT_VERSION_MAJOR}" STREQUAL "5")
    target_link_libraries(${LIB_NAME} PRIVATE
      PkgConfig::QGSettings
    )
  endif()

else()
  add_library(${LIB_NAME} SHARED
    ${base_SRCS}
    ${dci_SRCS}
    ${filesystem_SRCS}
    ${log_SRCS}
    ${settings_SRC}
    ${utils_SRC}
    ${glob_SRC}
  )
  target_link_libraries(
    ${LIB_NAME} PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Xml
  )
  target_link_libraries(${LIB_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::CorePrivate
  )
  if("${QT_VERSION_MAJOR}" STREQUAL "5")
    target_link_libraries(${LIB_NAME} PRIVATE
      PkgConfig::QGSettings
    )
  endif()
endif()
set_target_properties(${LIB_NAME} PROPERTIES
  VERSION ${CMAKE_PROJECT_VERSION}
  SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR}
  EXPORT_NAME Core
)

target_compile_definitions(${LIB_NAME} PUBLIC
    PREFIX="${DSG_PREFIX_PATH}"
    DSYSINFO_PREFIX="${DSYSINFO_PREFIX}"
)

target_compile_definitions(${LIB_NAME} PRIVATE
    LIBDTKCORE_LIBRARY
)

target_include_directories(${LIB_NAME} PRIVATE
  ${uchardet_INCLUDE_DIRS}
)
target_include_directories(${LIB_NAME} PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/util>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/dci>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/log>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/base>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/base/private>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/global>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/DtkCore>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/settings>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/filesystem>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

target_include_directories(${LIB_NAME} INTERFACE
  $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>
)

target_link_directories(${LIB_NAME} INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  $<INSTALL_INTERFACE:${LIBRARY_INSTALL_DIR}>
)

set(EnableCov CACHE BOOL OFF)
if (EnableCov)
    dtk_setup_code_coverage(${LIB_NAME})
endif()

dtk_check_and_add_definitions(${LIB_NAME}
    DEFS
    OS_VERSION_TEST_FILE
    LSB_RELEASE_TEST_FILE
    OS_RELEASE_TEST_FILE
    DEEPIN_VERSION_TEST_FILE
)

set(TOINSTALLBASE
  ../include/base/dobject.h
  ../include/base/dsingleton.h
  ../include/base/private/dobject_p.h
  ../include/base/derror.h
  ../include/base/dexpected.h
)
install(FILES ${TOINSTALLBASE} DESTINATION "${INCLUDE_INSTALL_DIR}")
install(FILES ${LOG_HEADER} DESTINATION "${INCLUDE_INSTALL_DIR}")
install(FILES ${SETTINGS_HEADERS} DESTINATION "${INCLUDE_INSTALL_DIR}")
install(FILES ${UTILS_HEADERS} DESTINATION "${INCLUDE_INSTALL_DIR}")

install(DIRECTORY ../include/dci/ DESTINATION "${INCLUDE_INSTALL_DIR}" FILES_MATCHING PATTERN "*.h")
install(DIRECTORY ../include/DtkCore/ DESTINATION "${INCLUDE_INSTALL_DIR}" FILES_MATCHING PATTERN "*")
install(DIRECTORY ../include/filesystem/ DESTINATION "${INCLUDE_INSTALL_DIR}" FILES_MATCHING PATTERN "*.h")
install(DIRECTORY ../include/global/ DESTINATION "${INCLUDE_INSTALL_DIR}" FILES_MATCHING PATTERN "*.h")

install(TARGETS ${LIB_NAME} EXPORT ${DtkCore}Targets
 DESTINATION ${LIBRARY_INSTALL_DIR})

install(EXPORT ${DtkCore}Targets
  FILE ${DtkCore}Targets.cmake
  NAMESPACE Dtk${DTK_VERSION_MAJOR}::
  DESTINATION ${CONFIG_CMAKE_INSTALL_DIR}
)
