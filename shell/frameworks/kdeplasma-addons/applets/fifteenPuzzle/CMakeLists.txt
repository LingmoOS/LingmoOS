ecm_install_icons(
    ICONS sc-apps-fifteenpuzzle.svgz
    DESTINATION ${KDE_INSTALL_ICONDIR}
)

plasma_install_package(package org.kde.plasma.fifteenpuzzle)

ecm_add_qml_module(fifteenpuzzleplugin URI org.kde.plasma.private.fifteenpuzzle)
target_sources(fifteenpuzzleplugin PRIVATE
    plugin/fifteenimageprovider.cpp
    plugin/fifteenpuzzleplugin.cpp
)
target_link_libraries(fifteenpuzzleplugin PRIVATE Qt::Quick Qt::Qml)
ecm_finalize_qml_module(fifteenpuzzleplugin)
