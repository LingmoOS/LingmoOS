set(wizard_SRCS
    bluewizard.cpp
    main.cpp
    pages/connect.cpp
    pages/discover.cpp
    pages/fail.cpp
    pages/pairing.cpp
    pages/success.cpp
    wizardagent.cpp
)

ecm_qt_declare_logging_category(wizard_SRCS
    HEADER bluedevil_wizard.h
    IDENTIFIER BLUEDEVIL_WIZARD_LOG
    CATEGORY_NAME org.kde.lingmo.bluedevil.wizard
    DESCRIPTION "BlueDevil (wizard)"
    EXPORT BLUEDEVIL
)

ki18n_wrap_ui(wizard_SRCS
    pages/connect.ui
    pages/discover.ui
    pages/fail.ui
    pages/pairing.ui
    pages/success.ui
)

add_executable(bluedevil-wizard ${wizard_SRCS})

target_link_libraries(bluedevil-wizard
    Qt::DBus
    Qt::Widgets

    KF6::BluezQt
    KF6::CoreAddons
    KF6::DBusAddons
    KF6::I18n
    KF6::Notifications
    KF6::WidgetsAddons
    KF6::WindowSystem
)

install(TARGETS bluedevil-wizard ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES org.kde.bluedevilwizard.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES pin-code-database.xml DESTINATION ${KDE_INSTALL_DATADIR}/bluedevilwizard)
