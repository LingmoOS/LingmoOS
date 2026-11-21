lingmo_add_client(timedate TARGET TimeDateClient
    SOURCES time_date.cpp
    HEADERS time_date.h
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
