set(configureprinter_SRCS
    ConfigurePrinterInterface.cpp
    ConfigurePrinterInterface.h
    Debug.cpp
    Debug.h
    PrinterPage.cpp
    PrinterPage.h
    PrinterOptions.cpp
    PrinterOptions.h
    PrinterBehavior.cpp
    PrinterBehavior.h
    ConfigureDialog.cpp
    ConfigureDialog.h
    ConfigurePrinter.cpp
    ConfigurePrinter.h
    main.cpp
)

ki18n_wrap_ui(configureprinter_SRCS
    PrinterOptions.ui
    PrinterBehavior.ui
)

qt_add_dbus_adaptor(configureprinter_SRCS
    org.kde.ConfigurePrinter.xml
    ConfigurePrinterInterface.h
    ConfigurePrinterInterface
)

add_executable(configure-printer ${configureprinter_SRCS})
target_link_libraries(configure-printer
    Qt6::Core
    Qt6::Widgets
    KF6::CoreAddons
    KF6::I18n
    KF6::IconThemes
    KF6::IconWidgets
    KF6::KIOCore
    KF6::KIOWidgets
    KF6::WidgetsAddons
    KF6::DBusAddons
    KF6::ConfigCore
    KF6::IconWidgets
    Cups::Cups
    kcupslib
)

install(TARGETS configure-printer DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS org.kde.ConfigurePrinter.desktop DESTINATION ${KDE_INSTALL_APPDIR})
