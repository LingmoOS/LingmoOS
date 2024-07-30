#.rest:
# FindEditorConfig
# --------------
#
# Try to find EditorConfig on this system.
#
# This will define the following variables:
#
# ``EditorConfig_FOUND``
#    True if inotify is available
# ``EditorConfig_LIBRARIES``
#    This has to be passed to target_link_libraries()
# ``EditorConfig_INCLUDE_DIRS``
#    This has to be passed to target_include_directories()

#=============================================================================
# SPDX-FileCopyrightText: 2017 Christoph Cullmann <cullmann@kde.org>
#
# SPDX-License-Identifier: BSD-2-Clause
#=============================================================================

find_path(EditorConfig_INCLUDE_DIRS editorconfig/editorconfig.h)

if(EditorConfig_INCLUDE_DIRS)
    find_library(EditorConfig_LIBRARIES NAMES editorconfig)
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(EditorConfig
        FOUND_VAR
            EditorConfig_FOUND
        REQUIRED_VARS
            EditorConfig_LIBRARIES
            EditorConfig_INCLUDE_DIRS
    )
    mark_as_advanced(EditorConfig_LIBRARIES EditorConfig_INCLUDE_DIRS)
    include(FeatureSummary)
    set_package_properties(EditorConfig PROPERTIES
        URL "https://editorconfig.org/"
        DESCRIPTION "EditorConfig editor configuration file support."
    )
else()
   set(EditorConfig_FOUND FALSE)
endif()

mark_as_advanced(EditorConfig_LIBRARIES EditorConfig_INCLUDE_DIRS) 
