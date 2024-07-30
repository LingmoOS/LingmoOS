ecm_add_qml_module(lingmoshellprivateplugin URI "org.kde.lingmo.private.shell" VERSION 2.0 GENERATE_PLUGIN_SOURCE)

target_sources(lingmoshellprivateplugin PRIVATE
    kcategorizeditemsviewmodels.cpp
    lingmoappletitemmodel.cpp
    widgetexplorer.cpp
)

target_link_libraries(lingmoshellprivateplugin PRIVATE
        Qt::Core
        Qt::Quick
        Qt::Qml
        Qt::Gui
        Qt::Widgets
        Qt::Quick
        Qt::Qml
        Lingmo::Lingmo
        Lingmo::LingmoQuick
        KF6::I18n
        KF6::WidgetsAddons
        KF6::NewStuffWidgets
        KF6::WindowSystem
        Lingmo::Activities
        KF6::TextWidgets
        KF6::Package
)

ecm_finalize_qml_module(lingmoshellprivateplugin)

install(FILES
    wallpaperplugin.knsrc
    plasmoids.knsrc
    DESTINATION ${KDE_INSTALL_KNSRCDIR}
)
