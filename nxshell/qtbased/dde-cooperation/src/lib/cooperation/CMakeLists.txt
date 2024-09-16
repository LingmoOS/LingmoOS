add_subdirectory(core)
if (CMAKE_SYSTEM MATCHES "Linux")
    add_subdirectory(transfer)
endif()
