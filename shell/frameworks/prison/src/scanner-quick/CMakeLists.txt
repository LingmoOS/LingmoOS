# SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

ecm_add_qml_module(prisonscannerquickplugin URI "org.kde.prison.scanner" VERSION 1.0 GENERATE_PLUGIN_SOURCE)
target_sources(prisonscannerquickplugin PRIVATE types.h)
target_link_libraries(prisonscannerquickplugin PRIVATE Qt6::Quick KF6::PrisonScanner)
ecm_finalize_qml_module(prisonscannerquickplugin DESTINATION ${KDE_INSTALL_QMLDIR})
