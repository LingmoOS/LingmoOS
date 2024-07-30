# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

# Boosting us a bit

if (NOT LINGMO_ACTIVITIES_LIBRARY_ONLY)
   find_package (Boost 1.49 REQUIRED)

   string (REGEX MATCH "1053.." BOOST_VERSION_BLACKLISTED ${Boost_VERSION})

   if (BOOST_VERSION_BLACKLISTED AND NOT LINGMO_ACTIVITIES_ENABLE_EXCEPTIONS)
      message (
         WARNING
         "Boost.Container 1.53 has issues when exceptions are disabled. "
         "We will set the LINGMO_ACTIVITIES_ENABLE_EXCEPTIONS option."
         )
      set (LINGMO_ACTIVITIES_ENABLE_EXCEPTIONS ON)
   endif ()
endif ()

if (LINGMO_ACTIVITIES_ENABLE_EXCEPTIONS)
   string (REPLACE "-fno-exceptions" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
   add_definitions (-fexceptions)
endif ()

# =======================================================
# Starting the actual project definition

# The libraries do not depend on any compile-time features
add_subdirectory (lib)

if (NOT LINGMO_ACTIVITIES_LIBRARY_ONLY)
   include_directories (
      ${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}
      )
   add_subdirectory (imports)
endif ()

add_subdirectory (cli)

ecm_qt_install_logging_categories(
    EXPORT LINGMO_ACTIVITIES
    FILE lingmo-activities.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
