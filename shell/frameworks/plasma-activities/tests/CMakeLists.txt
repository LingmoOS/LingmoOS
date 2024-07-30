# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

# add_subdirectory(slc-interface)

if (UNIX AND NOT APPLE)
    # Requires X11
    add_subdirectory(activities-model)
endif ()


