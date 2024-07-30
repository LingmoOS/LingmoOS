add_library(settings STATIC
    kscreensaversettings.cpp
    shell_integration.cpp
    wallpaper_integration.cpp
    appearancesettings.cpp
)
set_property(TARGET settings PROPERTY POSITION_INDEPENDENT_CODE ON)

kconfig_add_kcfg_files(settings GENERATE_MOC kscreensaversettingsbase.kcfgc)
target_link_libraries(settings
    Qt6::Quick
    KF6::ConfigQml
    KF6::GlobalAccel
    KF6::I18n
    KF6::Package
    KF6::XmlGui
)
