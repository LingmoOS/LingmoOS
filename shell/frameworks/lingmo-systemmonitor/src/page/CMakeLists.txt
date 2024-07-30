
add_library(LingmoSystemMonitorPage)

ecm_add_qml_module(LingmoSystemMonitorPage URI "org.kde.ksysguard.page" GENERATE_PLUGIN_SOURCE DEPENDENCIES "QtCore" "org.kde.ksysguard.faces")

target_sources(LingmoSystemMonitorPage PRIVATE
    FaceLoader.cpp
    FaceLoader.h
    FacesModel.cpp
    FacesModel.h
    PageDataModel.cpp
    PageDataModel.h
    PageDataObject.cpp
    PageDataObject.h
    PagesModel.cpp
    PagesModel.h
    PageSortModel.cpp
    PageSortModel.h
    WidgetExporter.cpp
    WidgetExporter.h
)

ecm_target_qml_sources(LingmoSystemMonitorPage SOURCES
    ColumnControl.qml
    Container.qml
    EditablePage.qml
    PageContents.qml
    PageEditor.qml
    PlaceholderRectangle.qml
    RowControl.qml
    SectionControl.qml
    PageDialog.qml
    EditablePageAction.qml
    FaceControl.qml
    FaceConfigurationPage.qml
    LoadPresetDialog.qml
    EditorToolBar.qml
    MoveButton.qml
    PageSortDialog.qml
    DialogLoader.qml
    MissingSensorsDialog.qml
    ConditionalLoader.qml
)

target_link_libraries(LingmoSystemMonitorPage PRIVATE Qt::DBus Qt::Quick KF6::ConfigCore KF6::I18n KF6::NewStuffCore KSysGuard::Sensors KSysGuard::SensorFaces)

install(TARGETS LingmoSystemMonitorPage ${KDE_INSTALL_DEFAULT_ARGS})

ecm_finalize_qml_module(LingmoSystemMonitorPage)
