add_subdirectory(data-transfer)
add_subdirectory(dde-cooperation)

if (CMAKE_SYSTEM MATCHES "Linux")
    add_subdirectory(dde-cooperation-transfer)
endif()

# launcher
# add_subdirectory("launcher")
