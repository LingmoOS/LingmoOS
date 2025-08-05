### configuration plugin
add_definitions(-DTRANSLATION_DOMAIN=\"ocean_style_config\")
set(kstyle_ocean_config_PART_SRCS oceanstyleconfig.cpp)

set(kstyle_ocean_config_PART_SRCS ${kstyle_ocean_config_PART_SRCS} oceanstyleconfigmodule.cpp)

kconfig_add_kcfg_files(kstyle_ocean_config_PART_SRCS ../oceanstyleconfigdata.kcfgc)
set(kstyle_ocean_config_PART_FORMS ui/oceanstyleconfig.ui)
ki18n_wrap_ui(kstyle_ocean_config_PART_FORMS_HEADERS ${kstyle_ocean_config_PART_FORMS})

add_library(kstyle_ocean_config MODULE
    ${kstyle_ocean_config_PART_SRCS}
    ${kstyle_ocean_config_PART_FORMS_HEADERS}
)

target_link_libraries(kstyle_ocean_config Qt5::Core Qt5::Gui Qt5::Widgets Qt5::DBus)
target_link_libraries(kstyle_ocean_config KF5::ConfigCore KF5::ConfigWidgets KF5::I18n)
install(TARGETS kstyle_ocean_config DESTINATION ${QT_PLUGIN_INSTALL_DIR})
install(FILES oceanstyleconfig.desktop DESTINATION  ${SERVICES_INSTALL_DIR})

########### next target ###############
find_package(KF5KCMUtils CONFIG)
set_package_properties(KF5KCMUtils PROPERTIES
    DESCRIPTION "KF5 KCM utils Framework"
    URL "https://projects.kde.org/projects/frameworks/kcmutils"
    TYPE OPTIONAL
    PURPOSE "Required for ocean-settings5 application to build")

if( KF5KCMUtils_FOUND )
    set(ocean_settings_SOURCES main.cpp)
    add_executable(ocean-settings5 ${ocean_settings_SOURCES} )
    target_link_libraries(ocean-settings5 Qt5::Core Qt5::Gui Qt5::Widgets )
    target_link_libraries(ocean-settings5 KF5::I18n KF5::KCMUtils)

    install(TARGETS ocean-settings5 ${INSTALL_TARGETS_DEFAULT_ARGS})

    # icon
    ecm_install_icons(ICONS sc-apps-ocean-settings.svgz
        DESTINATION ${ICON_INSTALL_DIR}
        THEME hicolor )
endif()
