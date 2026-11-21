find_package(LingmoClients REQUIRED COMPONENTS audio)
target_link_libraries(settings-ui PUBLIC Lingmo::AudioClient)
