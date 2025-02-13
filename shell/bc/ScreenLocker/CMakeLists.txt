cmake_minimum_required(VERSION 3.16)

project(screenlocker)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# set(CMAKE_AUTOUIC ON)
# set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTORCC ON)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(FeatureSummary)

find_package(Qt5 COMPONENTS Core DBus Widgets X11Extras Quick LinguistTools REQUIRED)
find_package(X11)

# ----------- Check --------------------

check_include_file("sys/prctl.h" HAVE_SYS_PRCTL_H)
check_symbol_exists(PR_SET_DUMPABLE "sys/prctl.h" HAVE_PR_SET_DUMPABLE)
check_include_file("sys/procctl.h" HAVE_SYS_PROCCTL_H)
check_symbol_exists(PROC_TRACE_CTL "sys/procctl.h" HAVE_PROC_TRACE_CTL)
if (HAVE_PR_SET_DUMPABLE OR HAVE_PROC_TRACE_CTL)
 set(CAN_DISABLE_PTRACE TRUE)
endif ()
add_feature_info("prctl/procctl tracing control"
                CAN_DISABLE_PTRACE
                "Required for disallowing ptrace on greeter and kcheckpass process")

check_include_file("sys/signalfd.h" HAVE_SIGNALFD_H)
if (NOT HAVE_SIGNALFD_H)
   check_include_files("sys/types.h;sys/event.h" HAVE_EVENT_H)
endif ()
add_feature_info("sys/signalfd.h"
                HAVE_SIGNALFD_H
                "Use the signalfd() api for signalhandling")
add_feature_info("sys/event.h"
                HAVE_EVENT_H
                "Use the kevent() and sigwaitinfo() api for signalhandling")

# --------------------------------------

option(PAM_REQUIRED "Require building with PAM" ON)

include(ConfigureChecks.cmake)

configure_file(config-workspace.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-workspace.h)
configure_file(config-unix.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-unix.h )
configure_file(config-X11.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-X11.h)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu90")

configure_file(config-screenlocker.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-screenlocker.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(screenlocker)
add_subdirectory(checkpass)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)


# Get the installation directory from qmake
get_target_property(QT_QMAKE_EXECUTABLE ${Qt5Core_QMAKE_EXECUTABLE} IMPORTED_LOCATION)
if(NOT QT_QMAKE_EXECUTABLE)
    message(FATAL_ERROR "qmake is not found.")
endif()

execute_process(COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_QML
    OUTPUT_VARIABLE INSTALL_QMLDIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(INSTALL_QMLDIR)
    message(STATUS "qml directory:" "${INSTALL_QMLDIR}")
else()
    message(FATAL_ERROR "qml directory cannot be detected.")
endif()

# Install translations
file(GLOB TS_FILES translations/*.ts)
foreach(filepath ${TS_FILES})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath})
    list(APPEND ts_files_replaced ${filename})
endforeach()
qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${ts_files_replaced})
add_custom_target(translations DEPENDS ${QM_FILES} SOURCES ${ts_files_replaced})
add_dependencies(lingmo-screenlocker translations)
install(FILES ${QM_FILES} DESTINATION /usr/share/lingmo-screenlocker/translations)

