find_package(LingmoClients REQUIRED COMPONENTS upower powerprofiles)
target_link_libraries(settings-ui PUBLIC Lingmo::UPowerClient Lingmo::PowerProfilesClient)
