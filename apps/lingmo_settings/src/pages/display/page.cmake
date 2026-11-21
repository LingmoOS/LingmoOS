find_package(LingmoClients REQUIRED COMPONENTS display)
target_link_libraries(settings-ui PUBLIC Lingmo::DisplayClient)
