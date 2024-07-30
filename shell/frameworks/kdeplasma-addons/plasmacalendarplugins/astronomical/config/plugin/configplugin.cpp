/*
    SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "configstorage.h"

#include <QQmlEngine>
#include <QQmlExtensionPlugin>

class AstronomicalConfigPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override
    {
        qmlRegisterType<ConfigStorage>(uri, 1, 0, "ConfigStorage");
    }
};

#include "configplugin.moc"
