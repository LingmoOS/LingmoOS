lingmo_add_client(powerprofiles TARGET PowerProfilesClient
    SOURCES power_profiles.cpp
    HEADERS power_profiles.h
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
