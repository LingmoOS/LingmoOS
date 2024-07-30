set(printmanagerkded_SRCS
    NewPrinterNotification.cpp
    PrintManagerKded.cpp
    NewPrinterNotification.h
    PrintManagerKded.h
)

qt_add_dbus_adaptor(printmanagerkded_SRCS
    com.redhat.NewPrinterNotification.xml
    NewPrinterNotification.h
    NewPrinterNotification
)

add_library(kded_printmanager MODULE ${printmanagerkded_SRCS})
set_target_properties(kded_printmanager PROPERTIES OUTPUT_NAME printmanager)

target_link_libraries(kded_printmanager PRIVATE
    Qt6::Core
    KF6::DBusAddons
    KF6::Notifications
    kcupslib
)

ecm_qt_declare_logging_category(kded_printmanager
    HEADER pmkded_log.h
    IDENTIFIER PMKDED
    CATEGORY_NAME org.kde.plasma.printmanager.kded
    DESCRIPTION "Print Manager (kded)"
    EXPORT PMLOGS
)

install(TARGETS kded_printmanager DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kded)
install(FILES printmanager.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})
