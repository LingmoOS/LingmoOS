# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

ecm_add_qml_module(knotificationqmlplugin URI org.kde.notification VERSION 1.0 GENERATE_PLUGIN_SOURCE)

target_sources(knotificationqmlplugin PRIVATE
    knotificationqmlplugin.cpp
)

target_link_libraries(knotificationqmlplugin PRIVATE
    KF6Notifications
    Qt6::Qml
)

ecm_finalize_qml_module(knotificationqmlplugin)
