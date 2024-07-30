kde_enable_exceptions()

add_definitions(-DTRANSLATION_DOMAIN=\"panelspacer\")

kcoreaddons_add_plugin(org.kde.lingmo.panelspacer SOURCES panelspacer.cpp panelspacer.h INSTALL_NAMESPACE "lingmo/applets")

target_link_libraries(org.kde.lingmo.panelspacer Qt::Gui Qt::Core Qt::Qml Qt::Quick Lingmo::Lingmo Lingmo::LingmoQuick KF6::I18n)
