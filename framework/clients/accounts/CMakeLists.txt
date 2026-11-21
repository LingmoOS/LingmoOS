lingmo_add_client(accounts TARGET AccountsClient
    SOURCES accounts_manager.cpp user_account.cpp
    HEADERS accounts_manager.h user_account.h
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
