include(GenerateExportHeader)

set(libkcups_SRCS
    KIppRequest.cpp
    KCupsPasswordDialog.cpp
    KCupsConnection.cpp
    KCupsJob.cpp
    KCupsPrinter.cpp
    KCupsServer.cpp
    KCupsRequest.cpp
    ClassListWidget.cpp
    PPDModel.cpp
    SelectMakeModel.cpp
    NoSelectionRectDelegate.cpp
    PrinterModel.cpp
    JobModel.cpp
    JobSortFilterModel.cpp
    ProcessRunner.cpp
    DevicesModel.cpp

    KIppRequest.h
    KCupsPasswordDialog.h
    KCupsConnection.h
    KCupsJob.h
    KCupsPrinter.h
    KCupsServer.h
    KCupsRequest.h
    ClassListWidget.h
    PPDModel.h
    SelectMakeModel.h
    NoSelectionRectDelegate.h
    PrinterModel.h
    JobModel.h
    JobSortFilterModel.h
    ProcessRunner.h
    DevicesModel.h
)

ki18n_wrap_ui(libkcups_SRCS
    SelectMakeModel.ui
)

add_library(kcupslib SHARED ${libkcups_SRCS})
set_target_properties(kcupslib PROPERTIES
    SOVERSION ${PROJECT_VERSION}
    VERSION ${PROJECT_VERSION}
)

generate_export_header(kcupslib)

target_link_libraries(kcupslib PUBLIC
    Qt::Core
    Qt::DBus
    Qt::Qml
    Qt::Widgets
    KF6::ConfigWidgets
    KF6::CoreAddons
    KF6::I18n
    KF6::IconThemes
    KF6::KIOCore
    KF6::KIOWidgets
    KF6::WidgetsAddons
    KF6::WindowSystem
    Cups::Cups
)

ecm_add_qml_module(kcupslib URI "org.kde.plasma.printmanager" GENERATE_PLUGIN_SOURCE)
ecm_finalize_qml_module(kcupslib)

ecm_qt_declare_logging_category(kcupslib
    HEADER kcupslib_log.h
    IDENTIFIER LIBKCUPS
    CATEGORY_NAME org.kde.plasma.printmanager.libkcups
    DESCRIPTION "Print Manager (libkcups)"
    EXPORT PMLOGS
)

install(TARGETS kcupslib ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
