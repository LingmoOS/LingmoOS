find_package(LingmoClients REQUIRED COMPONENTS appearance)
find_package(PkgConfig REQUIRED)
pkg_check_modules(Xcursor REQUIRED IMPORTED_TARGET xcursor)
target_link_libraries(settings-ui PUBLIC Lingmo::AppearanceClient PkgConfig::Xcursor)
