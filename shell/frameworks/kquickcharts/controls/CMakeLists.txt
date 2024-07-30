
add_library(QuickChartsControls)
set_target_properties(QuickChartsControls PROPERTIES
    SOVERSION ${KF6QuickCharts_SOVERSION}
    VERSION ${KF6QuickCharts_VERSION}
)

ecm_add_qml_module(QuickChartsControls URI "org.kde.quickcharts.controls" VERSION 1.0 DEPENDENCIES QtCore QtQuick GENERATE_PLUGIN_SOURCE)

target_sources(QuickChartsControls PRIVATE
    AxisLabels.cpp
    AxisLabels.h
    GridLines.cpp
    GridLines.h
    LegendLayout.cpp
    LegendLayout.h
    LegendModel.cpp
    LegendModel.h
    LineGridNode.cpp
    LineGridNode.h
)

set_source_files_properties(Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

ecm_target_qml_sources(QuickChartsControls SOURCES
    Legend.qml
    LegendDelegate.qml
    LineChartControl.qml
    PieChartControl.qml
    Theme.qml
    KirigamiTheme.qml
)

target_link_libraries(QuickChartsControls PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::Qml
    Qt6::Gui
    Qt6::QuickControls2
    QuickCharts
)

ecm_finalize_qml_module(QuickChartsControls DESTINATION ${KDE_INSTALL_QMLDIR})

install(TARGETS QuickChartsControls ${KDE_INSTALL_DEFAULT_ARGUMENTS})
