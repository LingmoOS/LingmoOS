ecm_add_qml_module(OceanImpl URI "org.kde.ocean.impl"
    GENERATE_PLUGIN_SOURCE
    DEPENDENCIES
    "QtQuick.Controls"
    "org.kde.lingmoui"
)

target_sources(OceanImpl PRIVATE
    oceandial.cpp
    oceandial.h
    iconlabellayout.cpp
    iconlabellayout.h
    iconlabellayout_p.h
    kcolorutilssingleton.cpp
    kcolorutilssingleton.h
    paintedsymbolitem.cpp
    paintedsymbolitem.h
    paintedsymbolitem_p.h
    qquickicon.cpp
    qquickicon_p.h
)

set_source_files_properties(Units.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

ecm_target_qml_sources(OceanImpl SOURCES
    ButtonBackground.qml
    CheckIndicator.qml
    ComboBoxBackground.qml
    CursorDelegate.qml
    CursorHandle.qml
    DelegateBackground.qml
    FocusRect.qml
    IconLabelContent.qml
    IconLabelShortcutContent.qml
    InlineIconLabelContent.qml
    LargeShadow.qml
    ListViewHighlight.qml
    MediumShadow.qml
    MenuItemBackground.qml
    OverlayDimBackground.qml
    OverlayModalBackground.qml
    RadioIndicator.qml
    ScrollHandle.qml
    SliderGroove.qml
    SliderHandle.qml
    SmallBoxShadow.qml
    SpinBoxIndicator.qml
    SwitchIndicator.qml
    TextEditBackground.qml
    Theme.qml
    Units.qml
)

target_link_libraries(OceanImpl PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::QuickTemplates2
    KF6::ConfigCore
    KF6::GuiAddons
)

ecm_finalize_qml_module(OceanImpl)
