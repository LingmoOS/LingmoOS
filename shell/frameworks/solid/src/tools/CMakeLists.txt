add_subdirectory(solid-hardware)

if(WITH_NEW_SOLID_JOB AND WITH_NEW_POWER_ASYNC_API)
    add_subdirectory(solid-power)
endif()
