find_package(Qt6 REQUIRED COMPONENTS ShaderTools)

ecm_add_qml_module(graphicaleffects URI org.kde.graphicaleffects
    VERSION 1.0
    QML_FILES
        Lanczos.qml
        BadgeEffect.qml
    GENERATE_PLUGIN_SOURCE
)

qt_add_shaders(graphicaleffects "graphicaleffects_shaders"
    BATCHABLE
    PRECOMPILE
    OPTIMIZED
    PREFIX
        "/shaders"
    FILES
       "badge.frag"
       "preserveaspect.vert"
       "lanczos2sharp.frag"
)

ecm_finalize_qml_module(graphicaleffects)
