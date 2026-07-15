#.rst:
# FindQt6ThemeSupport
# --------------------
#
# Locate the private Qt 6 ThemeSupport module used by Lingmo's QPA platform
# theme plugin. Qt does not export this module as a standard CMake package,
# so it is resolved from the Qt6Gui pkg-config metadata.

find_package(PkgConfig REQUIRED)
pkg_check_modules(PKG_Qt6ThemeSupport QUIET Qt6Gui)

set(Qt6ThemeSupport_DEFINITIONS ${PKG_Qt6ThemeSupport_CFLAGS_OTHER})
set(Qt6ThemeSupport_VERSION ${PKG_Qt6ThemeSupport_VERSION})

find_path(Qt6ThemeSupport_INCLUDE_DIR
    NAMES QtThemeSupport/private/qgenericunixthemes_p.h
    HINTS
        ${PKG_Qt6ThemeSupport_INCLUDEDIR}/QtThemeSupport/${PKG_Qt6ThemeSupport_VERSION}/
)

find_library(Qt6ThemeSupport_LIBRARY
    NAMES Qt6ThemeSupport
    HINTS ${PKG_Qt6ThemeSupport_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Qt6ThemeSupport
    FOUND_VAR Qt6ThemeSupport_FOUND
    REQUIRED_VARS Qt6ThemeSupport_LIBRARY Qt6ThemeSupport_INCLUDE_DIR
    VERSION_VAR Qt6ThemeSupport_VERSION
)

if(Qt6ThemeSupport_FOUND AND NOT TARGET Qt6ThemeSupport::Qt6ThemeSupport)
    add_library(Qt6ThemeSupport::Qt6ThemeSupport UNKNOWN IMPORTED)
    set_target_properties(Qt6ThemeSupport::Qt6ThemeSupport PROPERTIES
        IMPORTED_LOCATION "${Qt6ThemeSupport_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${Qt6ThemeSupport_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${Qt6ThemeSupport_INCLUDE_DIR}"
    )
endif()

mark_as_advanced(Qt6ThemeSupport_LIBRARY Qt6ThemeSupport_INCLUDE_DIR)

set(Qt6ThemeSupport_LIBRARIES ${Qt6ThemeSupport_LIBRARY})
set(Qt6ThemeSupport_INCLUDE_DIRS ${Qt6ThemeSupport_INCLUDE_DIR})
set(Qt6ThemeSupport_VERSION_STRING ${Qt6ThemeSupport_VERSION})

include(FeatureSummary)
set_package_properties(Qt6ThemeSupport PROPERTIES
    URL "https://www.qt.io"
    DESCRIPTION "Qt 6 ThemeSupport private module"
)
