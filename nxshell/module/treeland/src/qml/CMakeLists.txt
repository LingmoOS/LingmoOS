set_source_files_properties(QmlHelper.qml
    PROPERTIES
        QT_QML_SINGLETON_TYPE TRUE
)

find_package(Qt6 COMPONENTS Quick QuickControls2 REQUIRED)
find_package(Dtk6 REQUIRED COMPONENTS Declarative)

qt_add_qml_module(treeland-qml
    URI TreeLand
    VERSION "1.0"
    STATIC
    QML_FILES
        Main.qml
        CloseAnimation.qml
        OutputDelegate.qml
        QmlHelper.qml
        StackToplevelHelper.qml
        StackWorkspace.qml
        TiledToplevelHelper.qml
        TiledWorkspace.qml
        WindowDecoration.qml
        SurfaceWrapper.qml
        WindowMenu.qml
        InputPopupSurface.qml
        LayerSurface.qml
        WindowsSwitcher.qml
        WindowsSwitcherPreview.qml
        DockPreview.qml
        MultitaskView.qml
        EQHGrid.qml
        ToplevelContainer.qml
        NewAnimation.qml
        SlideLayout.qml
        TallLayout.qml
        HorizontalLayout.qml
        VerticalLayout.qml
        WorkspaceManager.qml

    RESOURCE_PREFIX
        /qt/qml
    OUTPUT_DIRECTORY
        ${PROJECT_BINARY_DIR}/qt/qml/TreeLand
)

target_link_libraries(treeland-qml
    PUBLIC
        Waylib::WaylibServer
        Dtk6::Declarative
        Qt6::Quick
        Qt6::QuickControls2
)
