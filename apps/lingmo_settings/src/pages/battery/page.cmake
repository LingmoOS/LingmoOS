find_package(LingmoClients REQUIRED COMPONENTS upower)
target_link_libraries(settings-ui PUBLIC Lingmo::UPowerClient)
