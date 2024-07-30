# - Find CatDoc
#
# This module defines
#  Xattr_FOUND - whether the xattr header was Xattr_FOUND


# XATTR_FOUND libattr found
# XATTR_INCLUDE_DIRS include directories
# XATTR_LIBRARIES libattr library

 # Already in cache?
if (XATTR_INCLUDE_DIRS AND XATTR_LIBRARIES)
    set(XATTR_FOUND TRUE)
else ()

    find_path(XATTR_INCLUDE sys/xattr.h
        /usr/include
        /usr/local/include
        ${CMAKE_INCLUDE_PATH}
        ${CMAKE_INSTALL_PREFIX}/usr/include
    )

    find_library(XATTR_LIB NAMES attr libattr
        PATHS
            ${CMAKE_LIBRARY_PATH}
            ${CMAKE_INSTALL_PREFIX}/lib
    )

    if(XATTR_INCLUDE AND XATTR_LIB)
        set(XATTR_FOUND TRUE)
        set(XATTR_INCLUDE_DIRS ${XATTR_INCLUDE})
        set(XATTR_LIBRARIES ${XATTR_LIB})
        message(STATUS "Found xattr")
    else()
        set(XATTR_FOUND FALSE)
        set(XATTR_LIBRARIES "")
        message(STATUS "Unable to find xattr")
    endif()

    mark_as_advanced(XATTR_INCLUDE_DIRS XATTR_LIBRARIES)
endif()
