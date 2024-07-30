# - Find LibJXR
# Find the JXR library 
# This module defines
#  LIBJXR_INCLUDE_DIRS, where to find jxrlib/JXRGlue.h
#  LIBJXR_LIBRARIES, the libraries needed to use JXR
#
# Based on cmake code found at https://github.com/microsoft/vcpkg/blob/master/ports/jxrlib/FindJXR.cmake

find_path(LIBJXR_INCLUDE_DIRS
    NAMES JXRGlue.h
    PATH_SUFFIXES jxrlib
)
mark_as_advanced(LIBJXR_INCLUDE_DIRS)

include(SelectLibraryConfigurations)

find_library(LIBJPEGXR_LIBRARY NAMES jpegxr)
find_library(LIBJXRGLUE_LIBRARY NAMES jxrglue)

set(LIBJXR_LIBRARIES ${LIBJPEGXR_LIBRARY} ${LIBJXRGLUE_LIBRARY})
mark_as_advanced(LIBJXR_LIBRARIES)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibJXR DEFAULT_MSG LIBJXR_INCLUDE_DIRS LIBJXR_LIBRARIES)
