add_subdirectory(data-transfer)
add_subdirectory(ocean-cooperation)

if (CMAKE_SYSTEM MATCHES "Linux")
    add_subdirectory(ocean-cooperation-transfer)
endif()

# launcher
# add_subdirectory("launcher")
