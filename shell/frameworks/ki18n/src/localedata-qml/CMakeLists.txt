# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-FileCopyrightText: 2022 Julius Künzel <jk.kdedev@smartlab.uber.space>
# SPDX-License-Identifier: BSD-3-Clause

ecm_add_qml_module(ki18nlocaledataqmlplugin URI "org.kde.i18n.localeData" VERSION 1.0)

target_sources(ki18nlocaledataqmlplugin PRIVATE
    ki18nlocaledataqmlplugin.cpp
)
target_link_libraries(ki18nlocaledataqmlplugin PRIVATE
    Qt6::Qml
    KF6::I18nLocaleData
)

ecm_finalize_qml_module(ki18nlocaledataqmlplugin DESTINATION ${KDE_INSTALL_QMLDIR})
