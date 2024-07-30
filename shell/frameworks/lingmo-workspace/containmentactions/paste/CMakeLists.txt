kcoreaddons_add_plugin(lingmo_containmentactions_paste SOURCES paste.cpp INSTALL_NAMESPACE "lingmo/containmentactions")
target_link_libraries(lingmo_containmentactions_paste
    Qt::Gui
    Qt::Widgets
    Lingmo::Lingmo
    Lingmo::LingmoQuick
    KF6::KIOCore
)

set_target_properties(lingmo_containmentactions_paste PROPERTIES
    OUTPUT_NAME org.kde.paste
)

ecm_qt_declare_logging_category(lingmo_containmentactions_paste
    HEADER containmentactions_paste_debug.h
    IDENTIFIER CONTAINMENTACTIONS_PASTE_DEBUG
    CATEGORY_NAME org.kde.lingmo.containmentactions_paste
    DESCRIPTION "containment actions paste"
    EXPORT LINGMOWORKSPACE
)
