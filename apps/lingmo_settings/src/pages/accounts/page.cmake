find_package(LingmoClients REQUIRED COMPONENTS accounts)
find_library(SETTINGS_CRYPT_LIBRARY crypt REQUIRED)
target_link_libraries(settings-ui PUBLIC Lingmo::AccountsClient ${SETTINGS_CRYPT_LIBRARY})
