kcmutils_add_qml_kcm(kcm_bluetooth SOURCES bluetooth.cpp bluetooth.h)

kconfig_add_kcfg_files(kcm_bluetooth GENERATE_MOC ../settings/filereceiversettings.kcfgc)

target_link_libraries(kcm_bluetooth PRIVATE
    Qt::Gui
    Qt::Qml
    Qt::DBus

    KF6::BluezQt
    KF6::ConfigGui
    KF6::CoreAddons
    KF6::I18n
    KF6::KCMUtilsQuick
    KF6::KIOGui
    KF6::Service
)
