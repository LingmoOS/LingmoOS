find_package(LingmoClients REQUIRED COMPONENTS bluez)
target_link_libraries(settings-ui PUBLIC Lingmo::BluezClient)

if(BUILD_TESTING)
    # The page test's fake BlueZ answers sd-bus calls for any object path.
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(Systemd REQUIRED IMPORTED_TARGET libsystemd)
    cmake_language(DEFER CALL target_link_libraries settings-ui-tests PRIVATE PkgConfig::Systemd)
endif()
