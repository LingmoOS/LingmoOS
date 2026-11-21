find_package(LingmoClients REQUIRED COMPONENTS networkmanager)
target_link_libraries(settings-ui PUBLIC Lingmo::NetworkManagerClient)
