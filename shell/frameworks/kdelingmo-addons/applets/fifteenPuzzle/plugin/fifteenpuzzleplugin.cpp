/*
 * SPDX-FileCopyrightText: 2014 Jeremy Whiting <jpwhiting@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "fifteenimageprovider.h"

#include <QDebug>
#include <QQmlEngine>
#include <QQmlExtensionPlugin>

class FifteenPuzzlePlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override
    {
        // Do some dummy registration, otherwise the plugin will be ignored at runtime
        qmlRegisterTypeNotAvailable(uri, 0, 1, "FifteenPuzzle", QStringLiteral("fifteenpuzzle"));
    }
    void initializeEngine(QQmlEngine *engine, const char *uri) override
    {
        qDebug() << "initializeEngine called, uri is " << uri;
        engine->addImageProvider(QStringLiteral("fifteenpuzzle"), new FifteenImageProvider());
    }
};

#include "fifteenpuzzleplugin.moc"
