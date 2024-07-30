
find_package(Qt6Widgets REQUIRED)
set_package_properties(Qt6Widgets PROPERTIES TYPE REQUIRED PURPOSE "The charts example needs Qt Widgets so the Plasma desktop style works properly.")
find_package(KF6Kirigami2 REQUIRED)
set_package_properties(KF6Kirigami2 PROPERTIES TYPE REQUIRED PURPOSE "The charts example uses Kirigami for its UI.")
find_package(KF6Declarative REQUIRED)
set_package_properties(KF6Declarative PROPERTIES TYPE REQUIRED PURPOSE "The charts example uses items from KF6Declarative for its UI.")

add_executable(kquickcharts_example)
ecm_add_qml_module(kquickcharts_example URI "org.kde.quickcharts.example" GENERATE_PLUGIN_SOURCE)

target_sources(kquickcharts_example PRIVATE main.cpp)

ecm_target_qml_sources(kquickcharts_example SOURCES
    BarChart.qml
    ChartPage.qml
    HistoryProxySource.qml
    Legend.qml
    LineChart.qml
    Main.qml
    PieChart.qml
    RangeEditor.qml
)

target_link_libraries(kquickcharts_example PRIVATE Qt6::Quick Qt6::Widgets)

install(TARGETS kquickcharts_example DESTINATION ${KDE_INSTALL_BINDIR})
