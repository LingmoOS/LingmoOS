find_package(LingmoClients REQUIRED COMPONENTS appearance)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SettingsWallpaper REQUIRED IMPORTED_TARGET icu-i18n icu-uc)
target_link_libraries(settings-ui PUBLIC Lingmo::AppearanceClient PkgConfig::SettingsWallpaper)
