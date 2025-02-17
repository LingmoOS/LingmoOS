set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH} )
include(UnixAuth)
set_package_properties(PAM PROPERTIES DESCRIPTION "PAM Libraries"
                       URL "https://www.kernel.org/pub/linux/libs/pam/"
                       TYPE OPTIONAL
                       PURPOSE "Required for screen unlocking and optionally used by the KDM log in manager"
                      )
if(PAM_REQUIRED)
set_package_properties(PAM PROPERTIES TYPE REQUIRED)
endif()
include(CheckTypeSize)
include(FindPkgConfig)

if (PAM_FOUND)
    set(KDE4_COMMON_PAM_SERVICE "kde" CACHE STRING "The PAM service to use unless overridden for a particular app.")

    macro(define_pam_service APP)
        string(TOUPPER ${APP}_PAM_SERVICE var)
        set(cvar KDE4_${var})
        set(${cvar} "${KDE4_COMMON_PAM_SERVICE}" CACHE STRING "The PAM service for ${APP}.")
        mark_as_advanced(${cvar})
        set(${var} "\"${${cvar}}\"")
    endmacro(define_pam_service)

    define_pam_service(kscreensaver)
endif (PAM_FOUND)

check_include_files(unistd.h HAVE_UNISTD_H)

set(CMAKE_EXTRA_INCLUDE_FILES sys/socket.h)
