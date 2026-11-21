find_package(LingmoClients REQUIRED COMPONENTS appearance)
target_link_libraries(settings-ui PUBLIC Lingmo::AppearanceClient)
