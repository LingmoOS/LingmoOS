add_subdirectory(kns)

if(AppStreamQt_FOUND AND packagekitqt6_FOUND)
    add_subdirectory(appstream)
endif()
