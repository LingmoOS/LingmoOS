
add_library(QuickCharts)
set_target_properties(QuickCharts PROPERTIES
    SOVERSION ${KF6QuickCharts_SOVERSION}
    VERSION ${KF6QuickCharts_VERSION}
)

ecm_add_qml_module(QuickCharts URI "org.kde.quickcharts" VERSION 1.0 DEPENDENCIES QtQuick GENERATE_PLUGIN_SOURCE)

target_sources(QuickCharts PRIVATE
    BarChart.cpp
    BarChart.h
    Chart.cpp
    Chart.h
    ItemBuilder.cpp
    ItemBuilder.h
    LineChart.cpp
    LineChart.h
    PieChart.cpp
    PieChart.h
    RangeGroup.cpp
    RangeGroup.h
    XYChart.cpp
    XYChart.h
    datasource/ArraySource.cpp
    datasource/ArraySource.h
    datasource/ChartAxisSource.cpp
    datasource/ChartAxisSource.h
    datasource/ChartDataSource.cpp
    datasource/ChartDataSource.h
    datasource/ColorGradientSource.cpp
    datasource/ColorGradientSource.h
    datasource/HistoryProxySource.cpp
    datasource/HistoryProxySource.h
    datasource/MapProxySource.cpp
    datasource/MapProxySource.h
    datasource/ModelSource.cpp
    datasource/ModelSource.h
    datasource/SingleValueSource.cpp
    datasource/SingleValueSource.h
    scenegraph/BarChartMaterial.cpp
    scenegraph/BarChartMaterial.h
    scenegraph/BarChartNode.cpp
    scenegraph/BarChartNode.h
    scenegraph/LineChartMaterial.cpp
    scenegraph/LineChartMaterial.h
    scenegraph/LineChartNode.cpp
    scenegraph/LineChartNode.h
    scenegraph/LineSegmentNode.cpp
    scenegraph/LineSegmentNode.h
    scenegraph/PieChartMaterial.cpp
    scenegraph/PieChartMaterial.h
    scenegraph/PieChartNode.cpp
    scenegraph/PieChartNode.h
    scenegraph/SDFShader.cpp
    scenegraph/SDFShader.h
)

ecm_qt_declare_logging_category(QuickCharts
    HEADER charts_general_logging.h
    IDENTIFIER GENERAL
    CATEGORY_NAME kf.quickcharts.general
    DEFAULT_SEVERITY Warning
    DESCRIPTION "KQuickCharts Library - General"
    EXPORT KQuickCharts
)

ecm_qt_declare_logging_category(QuickCharts
    HEADER charts_datasource_logging.h
    IDENTIFIER DATASOURCE
    CATEGORY_NAME kf.quickcharts.datasource
    DEFAULT_SEVERITY Warning
    DESCRIPTION "KQuickCharts Library - Data Sources"
    EXPORT KQuickCharts
)

ecm_qt_declare_logging_category(QuickCharts
    HEADER charts_deprecated_logging.h
    IDENTIFIER DEPRECATED
    CATEGORY_NAME kf.quickcharts.deprecated
    DEFAULT_SEVERITY Warning
    DESCRIPTION "KQuickCharts Library - Deprecated API"
    EXPORT KQuickCharts
)

target_link_libraries(QuickCharts PUBLIC
    Qt6::Core
    Qt6::Qml
    Qt6::Quick
)

target_include_directories(QuickCharts PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/datasource
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(_extra_args DEBUGINFO)
else()
    set(_extra_args OPTIMIZED)
endif()

qt6_add_shaders(QuickCharts "shaders"
    BATCHABLE
    ZORDER_LOC 6
    PRECOMPILE
    PREFIX "/qt/qml/org/kde/quickcharts/shaders/"
    FILES
        shaders/barchart.vert
        shaders/barchart.frag
        shaders/linechart.vert
        shaders/linechart.frag
        shaders/piechart.vert
        shaders/piechart.frag
    OUTPUTS
        barchart.vert.qsb
        barchart.frag.qsb
        linechart.vert.qsb
        linechart.frag.qsb
        piechart.vert.qsb
        piechart.frag.qsb
    ${_extra_args}
)

ecm_generate_export_header(QuickCharts
    BASE_NAME QuickCharts
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

ecm_finalize_qml_module(QuickCharts DESTINATION ${KDE_INSTALL_QMLDIR})

ecm_qt_install_logging_categories(EXPORT KQuickCharts DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})

install(TARGETS QuickCharts ${KDE_INSTALL_DEFAULT_ARGUMENTS})
