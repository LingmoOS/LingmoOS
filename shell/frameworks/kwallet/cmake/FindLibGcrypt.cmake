#.rst
# FindLibGcrypt
# -------------
#
# Finds the Libgcrypt library.
#
# This will define the following variables:
#
# ``LIBGCRYPT_FOUND``
#     True if the requested version of gcrypt was found
# ``LIBGCRYPT_VERSION``
#     The version of gcrypt that was found
# ``LIBGCRYPT_INCLUDE_DIRS``
#     The gcrypt include directories
# ``LIBGCRYPT_LIBRARIES``
#     The linker libraries needed to use the gcrypt library

# SPDX-FileCopyrightText: 2014 Nicolás Alvarez <nicolas.alvarez@gmail.com>
#
# SPDX-License-Identifier: BSD-3-Clause

find_program(LIBGCRYPTCONFIG_SCRIPT NAMES libgcrypt-config)
if(LIBGCRYPTCONFIG_SCRIPT)
    execute_process(
        COMMAND "${LIBGCRYPTCONFIG_SCRIPT}" --prefix
        RESULT_VARIABLE CONFIGSCRIPT_RESULT
        OUTPUT_VARIABLE PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (CONFIGSCRIPT_RESULT EQUAL 0)
        set(LIBGCRYPT_LIB_HINT "${PREFIX}/lib")
        set(LIBGCRYPT_INCLUDE_HINT "${PREFIX}/include")
    endif()
endif()

find_library(LIBGCRYPT_LIBRARY
    NAMES gcrypt
    HINTS ${LIBGCRYPT_LIB_HINT}
)
find_path(LIBGCRYPT_INCLUDE_DIR
    NAMES gcrypt.h
    HINTS ${LIBGCRYPT_INCLUDE_HINT}
)

if(LIBGCRYPT_INCLUDE_DIR)
    file(STRINGS ${LIBGCRYPT_INCLUDE_DIR}/gcrypt.h GCRYPT_H REGEX "^#define GCRYPT_VERSION ")
    string(REGEX REPLACE "^#define GCRYPT_VERSION \"(.*)\".*$" "\\1" LIBGCRYPT_VERSION "${GCRYPT_H}")
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(LibGcrypt
    FOUND_VAR LIBGCRYPT_FOUND
    REQUIRED_VARS LIBGCRYPT_LIBRARY LIBGCRYPT_INCLUDE_DIR
    VERSION_VAR LIBGCRYPT_VERSION
)
if(LIBGCRYPT_FOUND)
    set(LIBGCRYPT_LIBRARIES ${LIBGCRYPT_LIBRARY})
    set(LIBGCRYPT_INCLUDE_DIRS ${LIBGCRYPT_INCLUDE_DIR})
endif()

mark_as_advanced(LIBGCRYPT_LIBRARY LIBGCRYPT_INCLUDE_DIR)

include(FeatureSummary)
set_package_properties(LibGcrypt PROPERTIES
    DESCRIPTION "A general purpose cryptographic library based on the code from GnuPG."
    URL "http://www.gnu.org/software/libgcrypt/"
)
