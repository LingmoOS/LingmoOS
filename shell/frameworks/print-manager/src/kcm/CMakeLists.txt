kcmutils_add_qml_kcm(kcm_printer_manager SOURCES
    printermanager.cpp
    printermanager.h
)

ecm_qt_declare_logging_category(kcm_printer_manager
    HEADER pmkcm_log.h
    IDENTIFIER PMKCM
    CATEGORY_NAME org.kde.plasma.printmanager.kcm
    DESCRIPTION "Print Manager (kcm)"
    EXPORT PMLOGS
)

target_link_libraries(kcm_printer_manager PRIVATE
    Qt6::DBus
    Qt6::Qml
    KF6::CoreAddons
    KF6::KCMUtilsQuick
    KF6::I18n
    kcupslib
)

if(SCP_INSTALL)
    target_sources(kcm_printer_manager PRIVATE scpinstaller.cpp)
    target_link_libraries(kcm_printer_manager PRIVATE PK::packagekitqt6)
endif()
