find_package(LingmoClients REQUIRED COMPONENTS appearance)
find_package(PkgConfig REQUIRED)
pkg_check_modules(FontConfig REQUIRED IMPORTED_TARGET fontconfig)
target_link_libraries(settings-ui PUBLIC Lingmo::AppearanceClient PkgConfig::FontConfig)
