find_package(PkgConfig REQUIRED)
pkg_check_modules(SettingsKeyboard REQUIRED IMPORTED_TARGET expat icu-i18n icu-uc)
target_link_libraries(settings-ui PUBLIC PkgConfig::SettingsKeyboard)
