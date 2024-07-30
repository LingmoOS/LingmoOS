/*
    SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "openconnectui.h"

#include <KPluginFactory>

class OpenConnectF5Ui : public OpenconnectUiPlugin
{
    Q_OBJECT
public:
    explicit OpenConnectF5Ui(QObject *parent = nullptr, const QVariantList &args = QVariantList())
        : OpenconnectUiPlugin(parent, args)
    {
    }
};

K_PLUGIN_CLASS_WITH_JSON(OpenConnectF5Ui, "plasmanetworkmanagement_openconnect_f5ui.json")

#include "openconnect_f5ui.moc"
