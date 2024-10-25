find_path(Libcrypt_INCLUDE_DIR NAMES "crypt.h")
find_library(Libcrypt_LIBRARY NAMES "crypt")

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
    Libcrypt
    FOUND_VAR
        Libcrypt_FOUND
    REQUIRED_VARS
        Libcrypt_LIBRARY
        Libcrypt_INCLUDE_DIR
)

if(Libcrypt_FOUND AND NOT TARGET Libcrypt::Libcrypt)
    add_library(Libcrypt::Libcrypt UNKNOWN IMPORTED)
    set_target_properties(Libcrypt::Libcrypt PROPERTIES
        IMPORTED_LOCATION "${Libcrypt_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${Libcrypt_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${Libcrypt_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(Libcrypt_LIBRARY Libcrypt_INCLUDE_DIR)

# Compatibility
set(Libcrypt_LIBRARIES ${Libcrypt_LIBRARY})
set(Libcrypt_INCLUDE_DIRS ${Libcrypt_INCLUDE_DIR})
