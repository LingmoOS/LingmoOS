# Helper macros for dealing with device backends.

# SPDX-FileCopyrightText: 2019 Pino Toscano <pino@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause

# "Begin" the device backends addition, to be called *before* all the order
# add_device_backend* macros
macro(add_device_backends_begin)
    set(ENABLED_DEVICE_BACKENDS)
endmacro()

# Macro to add a new device backend; a new cmake option is automatically added
# to disable the build of that backend, available as
# BUILD_DEVICE_BACKEND_$name.
macro(add_device_backend backend)
    _add_device_backend(${backend} "${CMAKE_CURRENT_SOURCE_DIR}")
endmacro()

# Internal implementation of add_device_backend, mostly to pass the right
# source directory (in case of a function is the file containing that
# function).
function(_add_device_backend backend source_dir)
    set(cmakelists_path "${source_dir}/src/solid/devices/backends/${backend}/CMakeLists.txt")
    if(NOT (EXISTS "${cmakelists_path}"))
        message(FATAL_ERROR "Missing CMakeLists.txt for backend ${backend}")
    endif()

    option(BUILD_DEVICE_BACKEND_${backend} "Build the backend ${backend}" ON)
    if(NOT BUILD_DEVICE_BACKEND_${backend})
        return()
    endif()

    set(backends ${ENABLED_DEVICE_BACKENDS})
    list(APPEND backends ${backend})
    set(ENABLED_DEVICE_BACKENDS ${backends} PARENT_SCOPE)
endfunction()

# After all the add_device_backend()s are called, create all the cmake
# material:
# - the BUILD_DEVICE_BACKEND_$name cmake variables, used to conditionally
#   add stuff depending whether a backend is built
# - a feature info, to show that that backend was enabled
macro(add_device_backends_cmake)
    foreach(backend ${ENABLED_DEVICE_BACKENDS})
        set(BUILD_DEVICE_BACKEND_${backend} TRUE)
        add_feature_info(${backend} TRUE "Solid device '${backend}' backend.")
    endforeach()
endmacro()

# After all the add_device_backend()s are called, create/collect all the
# material used for the build:
# - create a config-backends.h file in the current binary directory, with
#   BUILD_DEVICE_BACKEND_$name defines for all the built backends
# - fill the sources_var, and libs_var variables resp. with the sources,
#   and the libraries needed for all the built backends
macro(add_device_backends_build base_backends_dir sources_var libs_var)
    foreach(backend ${ENABLED_DEVICE_BACKENDS})
        string(APPEND config_backends_output "#define BUILD_DEVICE_BACKEND_${backend} 1\n")
        unset(backend_sources)
        unset(backend_libs)
        include("${base_backends_dir}/${backend}/CMakeLists.txt")
        foreach(source ${backend_sources})
            if(NOT (IS_ABSOLUTE ${source}))
                set(source "${base_backends_dir}/${backend}/${source}")
            endif()
            list(APPEND ${sources_var} "${source}")
        endforeach()
        list(APPEND ${libs_var} ${backend_libs})
    endforeach()
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/config-backends-tmp.h" "${config_backends_output}")
    configure_file("${CMAKE_CURRENT_BINARY_DIR}/config-backends-tmp.h"
                   "${CMAKE_CURRENT_BINARY_DIR}/config-backends.h" COPYONLY)
    set_source_files_properties("${CMAKE_CURRENT_BINARY_DIR}/config-backends-tmp.h"
                                "${CMAKE_CURRENT_BINARY_DIR}/config-backends.h"
                                PROPERTIES SKIP_AUTOGEN TRUE)
endmacro()
