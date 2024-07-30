
add_library(LingmoUIDelegates)
ecm_add_qml_module(LingmoUIDelegates URI "org.kde.lingmoui.delegates"
    GENERATE_PLUGIN_SOURCE
    DEPENDENCIES QtQuick org.kde.lingmoui.platform org.kde.lingmoui.primitives
)

ecm_target_qml_sources(LingmoUIDelegates SOURCES
    IconTitleSubtitle.qml
    TitleSubtitle.qml

    SubtitleDelegate.qml
    CheckSubtitleDelegate.qml
    RadioSubtitleDelegate.qml
    SwitchSubtitleDelegate.qml
)

set_target_properties(LingmoUIDelegates PROPERTIES
    VERSION     ${PROJECT_VERSION}
    SOVERSION   6
    EXPORT_NAME "LingmoUIDelegates"
)

include_static_dependencies(LingmoUIDelegates EXPORT LingmoUITargets)

ecm_finalize_qml_module(LingmoUIDelegates)

if (NOT BUILD_SHARED_LIBS)
    install(TARGETS LingmoUIDelegatesplugin EXPORT LingmoUITargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
endif()

install(TARGETS LingmoUIDelegates EXPORT LingmoUITargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
