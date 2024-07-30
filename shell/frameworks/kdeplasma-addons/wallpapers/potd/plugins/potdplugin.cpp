/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlExtensionPlugin>

#include "potdbackend.h"
#include "potdprovider.h"
#include "potdprovidermodel.h"

class PotdPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override
    {
        qmlRegisterType<PotdBackend>(uri, 1, 0, "PotdBackend");
        qmlRegisterType<PotdProviderModel>(uri, 1, 0, "PotdProviderModel");

        qmlRegisterUncreatableType<PotdBackend>(uri, 1, 0, "Global", QStringLiteral("Error: only enums"));
    }
};

#include "potdplugin.moc"
