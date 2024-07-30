set(iconexplorer_SRCS
    main.cpp
    iconmodel.cpp
    colorschemes.cpp
    sortfiltermodel.cpp
)

add_executable(iconexplorer ${iconexplorer_SRCS})
target_compile_definitions(iconexplorer PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(iconexplorer PRIVATE
    Qt::Quick
    Qt::Gui
    Qt::Widgets # for QDirModel
    Plasma::Plasma
    KF6::KIOWidgets
    KF6::ConfigWidgets
    KF6::I18n
    KF6::IconThemes
    Qt::DBus
)

ecm_add_qml_module(iconexplorer URI org.kde.iconexplorer)

ecm_target_qml_sources(iconexplorer SOURCES
    qml/Actions.qml
    qml/Comparison.qml
    qml/Iconexplorer.qml
    qml/GlobalMenuBar.qml
    qml/IconGridDelegate.qml
    qml/IconGrid.qml
    qml/IconMontage.qml
    qml/Menu.qml
    qml/Preview.qml
    qml/ResponsivePreview.qml
    qml/Screenshot.qml
    qml/Tools.qml
    qml/ToolsResponsive.qml
)

install(TARGETS iconexplorer ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(editorplugin)
