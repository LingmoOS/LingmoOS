find_package(LingmoClients REQUIRED COMPONENTS update)
target_link_libraries(settings-ui PUBLIC Lingmo::UpdateClient)
