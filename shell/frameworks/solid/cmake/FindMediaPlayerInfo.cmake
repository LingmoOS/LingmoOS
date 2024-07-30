# This module searches media-player info
# The following variables are provided:
#  MEDIAPLAYERINFO_FOUND  - TRUE if found
#  MEDIAPLAYERINFO_PATH - the path where the files can be found

set(XDG_DATA_DIRS_ENV $ENV{XDG_DATA_DIRS})  # if(ENV{..}) does not work for me
if(XDG_DATA_DIRS_ENV)
   find_path(MEDIAPLAYERINFO_PATH sony_psp.mpi
             PATHS ENV XDG_DATA_DIRS
             PATH_SUFFIXES "media-player-info" NO_DEFAULT_PATH
            )
else()
   set(XDG_DATA_DIRS "/usr/share")
   message(STATUS "Warning: environment variable XDG_DATA_DIRS not set, falling back to ${XDG_DATA_DIRS}")
   find_path(MEDIAPLAYERINFO_PATH sony_psp.mpi
             PATHS "${XDG_DATA_DIRS}"
             PATH_SUFFIXES "media-player-info" NO_DEFAULT_PATH
            )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MediaPlayerInfo  DEFAULT_MSG MEDIAPLAYERINFO_PATH )
