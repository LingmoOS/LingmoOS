#
# Try to find the FreeImage library and include path.
# Once done this will define
#
# FreeImage_FOUND
# FreeImage_INCLUDE_PATH
# FreeImage_LIBRARY
#

find_path(FreeImage_INCLUDE_DIR
    NAMES FreeImage.h
    PATHS
    /usr/include
    /usr/local/include
    /sw/include
    /opt/local/include
    ${CMAKE_INCLUDE_PATH}
    ${CMAKE_INSTALL_PREFIX}/include)

find_library(FreeImage_LIBRARY
    NAMES FreeImage freeimage
    PATHS
    /usr/lib64
    /usr/lib
    /usr/local/lib64
    /usr/local/lib
    /sw/lib
    /opt/local/lib
    ${CMAKE_LIBRARY_PATH}
    ${CMAKE_INSTALL_PREFIX}/lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FreeImage
    FOUND_VAR FreeImage_FOUND
    REQUIRED_VARS
    FreeImage_LIBRARY
    FreeImage_INCLUDE_DIR
)

if(FreeImage_FOUND)
    set(FreeImage_LIBRARIES ${FreeImage_LIBRARY})
    set(FreeImage_INCLUDE_DIRS ${FreeImage_INCLUDE_DIR})
endif()

if(FreeImage_FOUND AND NOT TARGET FreeImage::FreeImage)
    add_library(FreeImage::FreeImage UNKNOWN IMPORTED)
    set_target_properties(FreeImage::FreeImage PROPERTIES
        IMPORTED_LOCATION "${FreeImage_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${FreeImage_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(
    FreeImage_INCLUDE_DIR
    FreeImage_LIBRARY
)
