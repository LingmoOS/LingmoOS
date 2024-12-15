# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"lingmo-kcm_kwin_scripts\")

kcoreaddons_add_plugin(lingmo-kcm_kwin_scripts INSTALL_NAMESPACE plasma/kcms/systemsettings)

target_sources(lingmo-kcm_kwin_scripts PRIVATE
    module.cpp
    kwinscriptsdata.cpp
)

target_link_libraries(lingmo-kcm_kwin_scripts
    Qt${QT_MAJOR_VERSION}::DBus

    KF5::I18n
    KF5::KCMUtilsCore
    KF5::KCMUtils
    KF5::NewStuffWidgets
    KF5::Package
    KF5::QuickAddons
    KF5::ItemViews
)

kpackage_install_package(package lingmo-kcm_kwin_scripts kcms)
install(FILES lingmo-kwinscripts.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})
