set(addprinter_SRCS
    main.cpp
    AddPrinter.cpp
    GenericPage.cpp
    PageDestinations.cpp
    ChooseLpd.cpp
    ChooseSamba.cpp
    ChooseSerial.cpp
    ChooseSocket.cpp
    ChooseUri.cpp
    PageChoosePrinters.cpp
    PageChoosePPD.cpp
    PageAddPrinter.cpp
    AddPrinterAssistant.cpp

    AddPrinter.h
    GenericPage.h
    PageDestinations.h
    ChooseLpd.h
    ChooseSamba.h
    ChooseSerial.h
    ChooseSocket.h
    ChooseUri.h
    PageChoosePrinters.h
    PageChoosePPD.h
    PageAddPrinter.h
    AddPrinterAssistant.h
)

ki18n_wrap_ui(addprinter_SRCS
    PageDestinations.ui
    ChooseLpd.ui
    ChooseSamba.ui
    ChooseSerial.ui
    ChooseSocket.ui
    ChooseUri.ui
    PageChoosePrinters.ui
    PageChoosePPD.ui
    PageAddPrinter.ui
)

add_executable(kde-add-printer
    ${addprinter_SRCS}
)

target_link_libraries(kde-add-printer
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    KF6::CoreAddons
    KF6::I18n
    KF6::IconThemes
    KF6::IconWidgets
    KF6::KIOCore
    KF6::KIOWidgets
    KF6::ConfigCore
    kcupslib
    KF6::IconWidgets
)

ecm_qt_declare_logging_category(kde-add-printer
    HEADER kde-add-printer_log.h
    IDENTIFIER PM_ADD_PRINTER
    CATEGORY_NAME org.kde.plasma.printmanager.kde-add-printer
    DESCRIPTION "Print Manager (legacy Add Printer App)"
    EXPORT PMLOGS
)

install(TARGETS kde-add-printer DESTINATION ${KDE_INSTALL_BINDIR})
install(PROGRAMS org.kde.kde-add-printer.desktop DESTINATION ${KDE_INSTALL_APPDIR})
