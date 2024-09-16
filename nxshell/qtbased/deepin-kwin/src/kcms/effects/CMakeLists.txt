# KI18N Translation Domain for this library.
add_definitions(-DTRANSLATION_DOMAIN=\"deepin-kcm_kwin_effects\")

########### next target ###############

set(deepin-kcm_kwin_effects_PART_SRCS
    kcm.cpp
    effectsfilterproxymodel.cpp
    desktopeffectsdata.cpp
)

kcoreaddons_add_plugin(deepin-kcm_kwin_effects SOURCES ${deepin-kcm_kwin_effects_PART_SRCS} INSTALL_NAMESPACE "plasma/kcms/systemsettings")

target_link_libraries(deepin-kcm_kwin_effects
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

install(FILES deepin-kwineffect.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})
kpackage_install_package(package deepin-kcm_kwin_effects kcms)
