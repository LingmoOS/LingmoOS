add_executable(lingmo-emojier emojier.cpp)

ecm_add_qml_module(lingmo-emojier URI org.kde.lingmo.emoji.app)

ecm_target_qml_sources(lingmo-emojier SOURCES
    ui/Emojier.qml
    ui/CategoryAction.qml
    ui/CategoryPage.qml
)

target_link_libraries(lingmo-emojier PRIVATE Qt::Widgets Qt::Quick KF6::ConfigGui KF6::I18n KF6::CoreAddons KF6::Crash KF6::DBusAddons KF6::WindowSystem Qt::GuiPrivate)

install(TARGETS lingmo-emojier ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES org.kde.lingmo.emojier.desktop DESTINATION ${KDE_INSTALL_DATADIR}/kglobalaccel)
install(PROGRAMS org.kde.lingmo.emojier.desktop DESTINATION ${KDE_INSTALL_APPDIR} )
