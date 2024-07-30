
add_library(PlasmaSystemMonitorTable)
ecm_add_qml_module(PlasmaSystemMonitorTable URI "org.kde.ksysguard.table" GENERATE_PLUGIN_SOURCE DEPENDENCIES "QtCore")

target_sources(PlasmaSystemMonitorTable PRIVATE
    ColumnDisplayModel.cpp
    ColumnDisplayModel.h
    ColumnSortModel.cpp
    ColumnSortModel.h
    ComponentCacheProxyModel.cpp
    ComponentCacheProxyModel.h
    ProcessSortFilterModel.cpp
    ProcessSortFilterModel.h
    ReverseColumnsProxyModel.cpp
    ReverseColumnsProxyModel.h
)

ecm_target_qml_sources(PlasmaSystemMonitorTable SOURCES
    BaseCellDelegate.qml
    BaseTableView.qml
    TextCellDelegate.qml
    ColumnConfigurationDialog.qml
    FirstCellDelegate.qml
    LineChartCellDelegate.qml
    TableViewHeader.qml
    UserCellDelegate.qml
    KillDialog.qml
)

target_link_libraries(PlasmaSystemMonitorTable PRIVATE Qt::Quick KSysGuard::Sensors KSysGuard::ProcessCore KF6::CoreAddons KF6::ItemModels)

install(TARGETS PlasmaSystemMonitorTable ${KDE_INSTALL_DEFAULT_ARGS})

ecm_finalize_qml_module(PlasmaSystemMonitorTable)
