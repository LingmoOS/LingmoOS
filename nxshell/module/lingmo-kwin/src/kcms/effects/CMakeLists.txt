# KI18N Translation Domain for this library.
add_definitions(-DTRANSLATION_DOMAIN=\"lingmo-kcm_kwin_effects\")

########### next target ###############

set(lingmo-kcm_kwin_effects_PART_SRCS
    kcm.cpp
    effectsfilterproxymodel.cpp
    desktopeffectsdata.cpp
)

kcoreaddons_add_plugin(lingmo-kcm_kwin_effects SOURCES ${lingmo-kcm_kwin_effects_PART_SRCS} INSTALL_NAMESPACE "plasma/kcms/systemsettings")

target_link_libraries(lingmo-kcm_kwin_effects
    Qt${QT_MAJOR_VERSION}::DBus

    KF5::KCMUtils
    KF5::I18n
    KF5::KCMUtils
    KF5::NewStuff
    KF5::QuickAddons
    KF5::XmlGui

    kcmkwincommon
)

########### install files ###############

install(FILES lingmo-kwineffect.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})
kpackage_install_package(package lingmo-kcm_kwin_effects kcms)
