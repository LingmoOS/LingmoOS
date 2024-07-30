add_definitions(-DTRANSLATION_DOMAIN=\"plasmanetworkmanagement-kded\")

add_library(kded_networkmanagement MODULE)

target_sources(kded_networkmanagement PRIVATE
    bluetoothmonitor.cpp
    connectivitymonitor.cpp
    notification.cpp
    monitor.cpp
    passworddialog.cpp
    secretagent.cpp
    service.cpp
    bluetoothmonitor.h
    connectivitymonitor.h
    notification.h
    monitor.h
    passworddialog.h
    secretagent.h
    service.h
    modemmonitor.cpp
    modemmonitor.h
    pindialog.cpp
    pindialog.h
)

ki18n_wrap_ui(kded_networkmanagement
    passworddialog.ui
    pinwidget.ui
)

ecm_qt_declare_logging_category(kded_networkmanagement HEADER plasma_nm_kded.h IDENTIFIER PLASMA_NM_KDED_LOG CATEGORY_NAME org.kde.plasma.nm.kded DESCRIPTION "Plasma NM (kded)" EXPORT PLASMANM)


set_target_properties(kded_networkmanagement PROPERTIES OUTPUT_NAME networkmanagement)

target_link_libraries(kded_networkmanagement
    plasmanm_internal
    plasmanm_editor
    KF6::ConfigCore
    KF6::DBusAddons
    KF6::I18n
    KF6::Notifications
    KF6::Service
    KF6::Solid
    KF6::Wallet
    KF6::WindowSystem
    KF6::KIOGui
    KF6::ModemManagerQt
)

install(TARGETS kded_networkmanagement DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kded)

install(FILES networkmanagement.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})
