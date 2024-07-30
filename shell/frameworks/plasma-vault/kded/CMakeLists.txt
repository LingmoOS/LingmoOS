configure_file(config-plasma-vault.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-plasma-vault.h)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/engine
)

add_library(kded_plasmavault MODULE)

target_sources(kded_plasmavault PRIVATE
    engine/backend_p.cpp
    engine/backend_p.h
    engine/backends/cryfs/cryfsbackend.cpp
    engine/backends/cryfs/cryfsbackend.h
    engine/backends/encfs/encfsbackend.cpp
    engine/backends/encfs/encfsbackend.h
    engine/backends/gocryptfs/gocryptfsbackend.cpp
    engine/backends/gocryptfs/gocryptfsbackend.h
    engine/commandresult.cpp
    engine/commandresult.h
    engine/fusebackend_p.cpp
    engine/fusebackend_p.h
    engine/singleton_p.h
    engine/types.cpp
    engine/types.h
    engine/vault.cpp
    engine/vault.h
    service.cpp
    service.h
    ui/activitieslinkingwidget.cpp
    ui/activitieslinkingwidget.h
    ui/backendchooserwidget.cpp
    ui/backendchooserwidget.h
    ui/cryfscypherchooserwidget.cpp
    ui/cryfscypherchooserwidget.h
    ui/dialogdsl.cpp
    ui/dialogdsl.h
    ui/directorychooserwidget.cpp
    ui/directorychooserwidget.h
    ui/directorypairchooserwidget.cpp
    ui/directorypairchooserwidget.h
    ui/mountdialog.cpp
    ui/mountdialog.h
    ui/namechooserwidget.cpp
    ui/namechooserwidget.h
    ui/noticewidget.cpp
    ui/noticewidget.h
    ui/offlineonlywidget.cpp
    ui/offlineonlywidget.h
    ui/passwordchooserwidget.cpp
    ui/passwordchooserwidget.h
    ui/vaultconfigurationdialog.cpp
    ui/vaultconfigurationdialog.h
    ui/vaultcreationwizard.cpp
    ui/vaultcreationwizard.h
    ui/vaultdeletionwidget.cpp
    ui/vaultdeletionwidget.h
    ui/vaultimportingwizard.cpp
    ui/vaultimportingwizard.h
    ui/vaultwizardbase.h

    ../common/vaultinfo.cpp
    ../common/vaultinfo.h
)

ki18n_wrap_ui(kded_plasmavault
    ui/activitieslinkingwidget.ui
    ui/backendchooserwidget.ui
    ui/cryfscypherchooserwidget.ui
    ui/directorychooserwidget.ui
    ui/directorypairchooserwidget.ui
    ui/mountdialog.ui
    ui/namechooserwidget.ui
    ui/noticewidget.ui
    ui/offlineonlywidget.ui
    ui/passwordchooserwidget.ui
    ui/vaultconfigurationdialog.ui
    ui/vaultcreationwizard.ui
    ui/vaultdeletionwidget.ui
    ui/vaultimportingwizard.ui
)

set_target_properties(kded_plasmavault
    PROPERTIES
        OUTPUT_NAME plasmavault
)

target_link_libraries(kded_plasmavault
    Qt::Core
    Qt::DBus
    Qt::Widgets

    Plasma::Activities
    KF6::ConfigCore
    KF6::ConfigWidgets
    KF6::CoreAddons
    KF6::DBusAddons
    KF6::I18n
    KF6::KIOCore
    KF6::KIOWidgets
    KF6::Service
    KF6::WidgetsAddons
    KSysGuard::ProcessCore
)
if (HAVE_NETWORKMANAGER)
    target_link_libraries(kded_plasmavault
        KF6::NetworkManagerQt
    )
endif()

install(TARGETS kded_plasmavault DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kded)
