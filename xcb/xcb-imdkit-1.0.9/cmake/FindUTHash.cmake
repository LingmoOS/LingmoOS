set(UTHASH_PREFIX "" CACHE PATH "Prefix uthash was installed into")

find_path(UTHash_INCLUDE_DIR NAMES uthash.h HINTS ${UTHASH_PREFIX} PATH_SUFFIXES include)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(UTHash
    FOUND_VAR
        UTHash_FOUND
    REQUIRED_VARS
        UTHash_INCLUDE_DIR
)

if(UTHash_FOUND AND NOT TARGET UTHash::UTHash)
    add_library(UTHash::UTHash INTERFACE IMPORTED )
    set_target_properties(UTHash::UTHash PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${EXECINFO_INCLUDE_DIR}"
    )
endif()
