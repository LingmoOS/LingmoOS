### configuration plugin
add_definitions(-DTRANSLATION_DOMAIN=\"ocean_style_config\")
set(kstyle_ocean_config_PART_SRCS oceanstyleconfig.cpp)

set(kstyle_ocean_config_PART_SRCS ${kstyle_ocean_config_PART_SRCS} oceanstyleconfigmodule.cpp)

kconfig_add_kcfg_files(kstyle_ocean_config_PART_SRCS ../oceanstyleconfigdata.kcfgc)
set(kstyle_ocean_config_PART_FORMS ui/oceanstyleconfig.ui)
ki18n_wrap_ui(kstyle_ocean_config_PART_FORMS_HEADERS ${kstyle_ocean_config_PART_FORMS})

kcoreaddons_add_plugin(oceanstyleconfig SOURCES
    ${kstyle_ocean_config_PART_SRCS}
    ${kstyle_ocean_config_PART_FORMS_HEADERS}
    INSTALL_NAMESPACE "kstyle_config"
)

target_link_libraries(oceanstyleconfig
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

if(HAVE_QTDBUS)
    target_link_libraries(oceanstyleconfig
        PUBLIC
            Qt6::DBus
    )
endif()

target_link_libraries(oceanstyleconfig
    PUBLIC
        KF6::CoreAddons
        KF6::ConfigCore
        KF6::ConfigWidgets
        KF6::I18n
)
if(QT_MAJOR_VERSION STREQUAL "6")
    target_link_libraries(oceanstyleconfig
        PUBLIC
    KF6::KCMUtils
)
endif()
kcmutils_generate_desktop_file(oceanstyleconfig)

set(ocean_settings_SOURCES main.cpp)
add_executable(ocean-settings ${ocean_settings_SOURCES} )
target_link_libraries(ocean-settings Qt6::Core Qt6::Gui Qt6::Widgets )
target_link_libraries(ocean-settings KF6::I18n KF6::KCMUtils KF6::CoreAddons)

set_target_properties(ocean-settings PROPERTIES OUTPUT_NAME ocean-settings6)

install(TARGETS ocean-settings ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

# icon
ecm_install_icons(ICONS sc-apps-ocean-settings.svgz
    DESTINATION ${KDE_INSTALL_ICONDIR}
    THEME hicolor )
